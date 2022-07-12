/**
 * Backbone JS App to manage the Events Request page/table
 */
(function($) {
	Drupal.behaviors.event_requests = {
		attach: function(context, settings) {
			// console.log('event_requests start of attach().', {
			// 	context: context,
			// 	settings: settings,
			// 	this: this
			// });

			//console.log('starting run of event_requests.js');
			var is_coordinator = Drupal.settings.event_requests.is_coordinator;
			var refreshRate = $.cookie('eaton_backbone_table_refresh_rate');
			var pollTimer = null;

			// @credit: https://mathiasbynens.be/notes/localstorage-pattern
			// Feature detect + local reference
			var hasStorage = (function() {
				var mod = 'modernizr';
				try {
					localStorage.setItem(mod, mod);
					localStorage.removeItem(mod);
					return true;
				} catch (exception) {
					return false;
				}
			}());

			window.loadingIndicator.show();

			$('.table-refresh-container select').change(function() {
				$.cookie('eaton_backbone_table_refresh_rate', $(this).val());
				if ($(this).val() == 0 && typeof(pollTimer) != "undefined") {
					clearTimeout(pollTimer);
				} else {
					pollTimer = setTimeout(pollForContent, +$(this).val());
				}
			})
			if (refreshRate != null) {
				$(".table-refresh-container select").val(refreshRate).change().trigger('chosen:updated');
			}
			// Clicks outside dialogs trigger its closing
			$(document).on('click', '.ui-widget-overlay', function(e) {
				if (typeof(appState) != 'undefined' && appState.hasOwnProperty('dialog')) {
					e.preventDefault();
					e.stopPropagation();
					appState.dialog.dialog('close');
				}
			})
			// Define some things
			var config = {
				nodeTemplateSelector: is_coordinator ? '#eaton-backbone-event-request-coordinator-node-template' : '#eaton-backbone-event-request-initiator-node-template',
				viewName: is_coordinator ? 'event_requests' : 'event_requests_initiator',
				itemsPerPageOptions: [5, 10, 15, 25, 50],
				itemsPerPage: 10, // default
				defaultDialogButtons: [{
					text: "OK",
					click: function() {
						$(this).dialog('close');
					}
				}],
				fns: {
					handleError: function(model, xhr, options) {
						window.loadingIndicator.hide();
						var html = 'An error occured while trying to process your request.  Please try again or request assistance from the system administrator.';
						config.fns.launchMessageDialog(html, 'warning');
					},
					launchDialog: function(options, html) {
						var dialog = appState.dialog;
						if (dialog.dialog('isOpen')) {
							dialog.dialog('close');
						}
						// reset default options
						var defaults = {
							modal: true,
							show: {
								effect: 'blind',
								duration: 300
							},
							hide: {
								effect: 'blind',
								duration: 300
							},
							open: function() {},
							close: function() {}
						};
						dialog.dialog('option', defaults);
						if (options != undefined) {
							dialog.dialog('option', options);
						}
						if (html != undefined) {
							dialog.html(html);
						}

						dialog.dialog('open');
					},
					launchMessageDialog: function(msg, type, closeCallback) {
						var type = type || 'info';
						var dialog = appState.dialog;
						var glyphicon = 'glyphicon-info-sign';
						switch (type) {
							case 'warning':
								glyphicon = 'glyphicon-warning-sign';
								break;
							case 'success':
								glyphicon = 'glyphicon-ok circle';
								break;
						}
						var markup = '<i class="alert-icon glyphicon ' + glyphicon + '"></i><h4>' + msg + '</h4>';
						dialog.html(markup);
						dialog.dialog('option', {
							'buttons': [],
							modal: false,
							width: 'auto',
							show: {
								effect: 'fade',
								duration: 300
							},
							hide: {
								effect: 'fade',
								duration: 300
							},
							open: function() {
								$(this).parents('.ui-dialog').addClass('message-dialog').addClass(type + '-dialog');
							},
							close: function() {
								$(this).parents('.ui-dialog').removeClass('message-dialog').removeClass(type + '-dialog');
								if (closeCallback != null) {
									closeCallback.call(this);
								}
							}
						});
						dialog.dialog('open');
					}
				},
				timeEntrySettings: {
					fromTo: false,
					show24Hours: true,
					showSeconds: false,
					spinnerImage: "",
					timeSteps: [1, 15, 0],
					timeFormat: 'HH:mm',
					interval: 15,
					forceRoundTime: true
				},
				datepickerSettings: {
					dateFormat: 'yy-mm-dd',
					minDate: 0,
					showButtonPanel: true,
					fixFocusIE: false,
					onSelect: function(dateText, inst) {
						this.fixFocusIE = true;
						$(this).change().focus();
					},
					onClose: function(dateText, inst) {
						this.fixFocusIE = true;
						this.focus();
					},
					beforeShow: function(input, inst) {
						var browserIsIE = jQuery.browser.msie || (jQuery.browser.mozilla && jQuery.browser.version.substr(0, 3) == "11.")
						if ($("#timepicker").is(':visible')) {
							return false;
						}
						var result = browserIsIE ? !this.fixFocusIE : true;
						this.fixFocusIE = false;
						return result;
					}
				}
			};
			// Create object to hold application state
			var appState = {
				showing: 0,
				interval: 0,
				page: 1,
				filter: [],
				dialog: {},
				touched_location: false,
				touched_mindate: false
			};

			// build dialog box, store in appState object
			appState.dialog = $("#modal").dialog({
				height: "auto",
				width: 400,
				modal: true,
				resizable: false,
				autoOpen: false,
				show: {
					effect: 'blind',
					duration: 300
				},
				hide: {
					effect: 'blind',
					duration: 300
				},
				buttons: config.defaultDialogButtons
			});

			// Fill in select options
			var itemsPerPageOptions = '';
			for (var z = 0, opt; opt = config.itemsPerPageOptions[z]; z++) {
				var htmlAttributes = '';
				if (opt == config.itemsPerPage) {
					htmlAttributes += ' selected';
				}
				itemsPerPageOptions += '<option value="' + opt + '"' + htmlAttributes + '>' + opt + '</option>';
			}
			$("#eaton-event-requests-items-per-page").html(itemsPerPageOptions);

			/*
			 * CSRF fix from https://groups.drupal.org/node/358308
			 */
			$.ajax({
				url: "/services/session/token",
				type: "get",
				dataType: "text",
				error: function(jqXHR, textStatus, errorThrown) {
					alert(errorThrown);
				},
				success: function(token) {
					$.ajaxSetup({
						beforeSend: function(request) {
							request.setRequestHeader("X-CSRF-Token", token);
						}
					});
				}
			});


			/****************************************
			 * NodeCollection from NodeView
			 *
			 * Provided by Drupal.Backbone
			 ****************************************/
			var NodeCollection = new Drupal.Backbone.Collections.NodeView({
				viewName: config.viewName
			});

			NodeCollection.sortKey = 'created';
			NodeCollection.sortOrder = 'desc';
			NodeCollection.comparator = compare;
			NodeCollection.filterBy = filterBy;
			NodeCollection.sortByField = sortByField;

			/////////
			
			function compare(model) {
				var output = 0;
				var val;
				if (this.sortKey.substr(0, 20) == 'field_possible_dates') {
					val = model.get(this.sortKey.substr(0, 20));
					if (val.hasOwnProperty('und')) {
						var i = parseInt(this.sortKey.substr(21, this.sortKey.length - 1)) - 1;
						if (typeof(val.und[i]) != "undefined") {
							output = val.und[i].value
						}
					}
				} else {
					val = model.get(this.sortKey);
					if (typeof(val) != "undefined") {
						if (typeof(val.und) != "undefined") {
							output = val.und[0].value;
						} else {
							output = val;
						}
					}
				}

				if (this.sortOrder == 'desc') {
					if (jQuery.isNumeric(output)) {
						output = -output;
					} else {
						str = output.toLowerCase();
						output = output.split("");
						output = _.map(output, function(letter) {
							return String.fromCharCode(-(letter.charCodeAt(0)));
						});
					}
				}
				return output;
			}
			
			function filterBy(field, val) {
				window.loadingIndicator.show();
				var _this = this;
				setTimeout(function() {
					if (val == -1) {
						// any, reset
						if (appState.filter.length > 1) {
							var find = appState.filter.filter(function(obj, idx) {
								return obj.field === field;
							});
							if (find.length) {
								var idx = appState.filter.indexOf(find[0]);
								appState.filter.splice(idx, 1)
							}
						} else {
							appState.filter = [];
						}
					} else {
						var found = false;
						if (appState.filter.length) {
							var find = appState.filter.filter(function(obj, idx) {
								return obj.field === field;
							});
							if (find.length) {
								found = true;
								var idx = appState.filter.indexOf(find[0]);
								appState.filter[idx].val = val;
							}
						}
						if (!found) {
							appState.filter.push({
								"field": field,
								"val": val
							});
						}
					}

					pollForContent();
				},200);
			}

			function sortByField(key) {
				this.sortKey = key;
				this.sort();
			}

			/**************************
			 * NodeView
			 **************************/

			// Create Model View (for node model)
			var NodeView = Drupal.Backbone.View.extend({
				templateSelector: config.nodeTemplateSelector,
				tagName: 'tr',
				initialize: function() {
					this.listenTo(this.model, 'change', this.render);
					this.listenTo(this.model, 'destroy', this.remove);
					Drupal.Backbone.View.prototype.initialize.apply(this);
				},
				render: function() {
					//console.log('NodeView.render()');
					Drupal.Backbone.View.prototype.render.call(this);
					var nid = this.model.get('nid');
					// add link to receipt
					if (Drupal.settings.event_requests.receipts[nid] != undefined) {
						var $date_created = this.$el.find('td.views-field-created');
						var date_created = $date_created.html();
						$date_created.html('<a target="_blank" href="/receipt/' + Drupal.settings.event_requests.receipts[nid] + '" title="View Receipt">' + date_created + '</a>');
					}
					var status = this.model.get('field_request_status').und[0].value;
					if (status == 'approved' || status == 'denied') {
						var $request_status = this.$el.find('td.views-field-request-status');
						var rs = $request_status.html();
						var type = status == 'approved' ? 'Approval' : 'Denial';
						$request_status.html('<a target="_blank" href="/email/' + Drupal.settings.event_requests.emails[nid] + '" title="View ' + type + ' Email">' + rs + '</a>');
					}
					if (status == 'approved') {
						// Add ICS link
						this.$el.find('.views-field-request-status').append('<a href="/ics/' + this.model.get('nid') + '" class="ics-link" title="Download ICS file and Add to your Calendar"></a>');
						// Highlight approved date
						var approved_date = this.model.get('field_approved_date');
						if (approved_date.hasOwnProperty('und')) {
							approved_date = approved_date.und[0];

							var possible_dates = this.model.get('field_possible_dates');
							var match = possible_dates.und.filter(function(date) {
								return date.value == approved_date.value && date.value2 == approved_date.value2;
							});
							if (match.length) {
								var idx = possible_dates.und.indexOf(match[0]) + 1;
								var $cell = this.$el.find('.views-field-possible-date-' + idx);
								$cell.addClass('selected');
							} else {
								// check denied suggested date
								var denied_suggested = this.model.get('field_denied_suggested_date');
								if (denied_suggested.hasOwnProperty('und')) {
									denied_suggested = denied_suggested.und[0];
									if (denied_suggested.value == approved_date.value && denied_suggested.value2 == approved_date.value2) {
										this.$el.find('.views-field-denied-suggested-date').addClass('selected');
									}
								}
							}
						}
					} else if (status == 'denied') {
						var suggested_accepted = this.model.get('suggested_date_accepted');
						if (suggested_accepted != undefined) {
							var $field = this.$el.find('.views-field-denied-suggested-date');
							$field.addClass('accepted');
							var accepted_isoString = new Date(suggested_accepted.timestamp * 1000).toISOString();
							var accepted_timeago = '<time class="timeago" datetime="' + accepted_isoString + '">' + suggested_accepted + '</time>';
							$field.prepend('<span class="td-annotation">Accepted ' + accepted_timeago + '</span>');
						}
					}
					return this;
				},
				events: {
					'click .approve-button': 'beginRequestApprove',
					'click .unapprove-button': 'unapproveEventRequest',
					'click .deny-button': 'beginRequestDeny',
					'click .delete-button': 'deleteRequestConfirm'
				},
				isApproved: function() {
					var field_request_status = this.model.get('field_request_status');
					return field_request_status.und != null && field_request_status.und[0].value == "approved";
				},
				isRequestedPrivate: function() {
					var field_request_private = this.model.get('field_request_private_event');
					return field_request_private.und != null && field_request_private.und[0].value == '1';
				},
				isDenied: function() {
					var field_request_status = this.model.get('field_request_status');
					return field_request_status.und != null && field_request_status.und[0].value == "denied";
				},
				isPrivate: function() {
					var field_event_visibility = this.model.get('field_event_visibility');
					return field_event_visibility.und != null && field_event_visibility.und[0].value == 'private';
				},
				prepareToSave: function() {
					// prepare data to PUT to the drupal server.  drupal services is finicky with how field data is saved.
					this.fixDates('field_possible_dates');
					this.fixDates('field_denied_on_date');
					this.fixDates('field_denied_suggested_date');
					this.fixLists();
					// DONT save these fields
					this.model.setNoSaveAttributes(
						['field_customer_boolean', 'field_event_visibility', 'field_event_attendees', 'field_purpose_of_visit', 'field_psec_support', 'field_psec_support_length',
							'field_psec_support_portion', 'field_additional_areas', 'field_special_request', 'field_psec_catering', 'field_visitor_information', 'field_visitor_type',
							'field_visitor_company_name', 'field_visitor_vista_number', 'field_vista_unknown', 'field_register_initiator', 'field_request_private_event',
							'suggested_date_accepted'
						]
					);

					// The event location has to be saved in this way or errors are thrown OR the event location is reset to drupal's default
					var loc = this.model.get('field_event_location');
					loc.und[0] = loc.und[0].value;
					this.model.set('field_event_location', loc);
				},
				fixLists: function() {
					// Go through room requirements and fix lists for save
					var node = this.model;
					var lists = node.get('field_room_requirements');
					if (typeof(lists.und) != "undefined") {
						var vals = [];
						for (var x = 0, req; req = lists.und[x]; x++) {
							vals.push(req.value);
						}
						node.set('field_room_requirements', {
							und: vals
						});
					}
				},
				fixDates: function(field_name) {
					var field_name = field_name || 'field_possible_dates';
					// Go through dates and fix for save
					var dates = this.getDates(field_name);
					if (dates.hasOwnProperty('und') && dates.und != null) {
						for (var x = 0, date; date = dates.und[x]; x++) {
							if (!_.isObject(date.value)) {
								var start = Date.parse(date.value);
								date.value = {
									'date': start.toString('yyyy-MM-dd'),
									'time': start.toString('HH:mm')
								};
								if (date.hasOwnProperty('value2')) {
									var end = Date.parse(date.value2);
									date.value2 = {
										'date': end.toString('yyyy-MM-dd'),
										'time': end.toString('HH:mm')
									};
								}
							}
						}
					}
				},
				// Helper function to return the possible dates field values
				getDates: function(field_name) {
					var field_name = field_name || 'field_possible_dates';
					return this.model.get(field_name);
				},
				beginRequestApprove: function(e) {
					if (typeof e != "undefined") {
						e.preventDefault();
						e.stopPropagation();
					}
					var node = this.model;

					var dates = this.getDates();
					var options = '';
					for (var x = 0, date; date = dates.und[x]; x++) {
						options += '<option value="' + x + '">' + dateRange(date) + '</option>';
					}
					// was suggested date accepted?
					var accepted = node.get('suggested_date_accepted');
					// add suggested date if available
					var suggested = node.get('field_denied_suggested_date');
					if (typeof(suggested.und) != "undefined") {
						if (accepted != undefined) {
							options += '<option value="suggested" selected>Suggested Date: ' + dateRange(suggested.und[0]) + '</option>';
						} else {
							options += '<option value="suggested">Suggested Date: ' + dateRange(suggested.und[0]) + '</option>';
						}
					}

					// check if this request was requested for private
					var requested_private = this.isRequestedPrivate();

					// store this NodeView in appState.
					appState.lastView = this;
					// configure dialog
					var dialog = appState.dialog;
					dialog.dialog('option', {
						width: "auto",
						'title': 'Choose a date to approve',
						'buttons': [{
							text: "Approve",
							click: function() {
								var approved_date = $(this).find('#choose-date').val();
								//  var has_end = approved_date.indexOf("(No End)") == -1 ? true : false;
								//  var all_day = approved_date.indexOf('(All Day)') != -1 ? true : false;
								var $approve_private = $(this).find('#approve-private-request');
								var approved_private = false;
								if ($approve_private.length) {
									if ($approve_private.attr('checked')) {
										approved_private = true;
									}
								}
								var date_value = approved_date;
								appState.lastView.approveEventRequest(date_value, approved_private);
							}
						}, {
							text: "Cancel",
							click: function() {
								$(this).dialog('close');
							}
						}],
						open: function() {
							dialog.parent().addClass('approve-dialog');
						},
						close: function() {
							dialog.parent().removeClass('approve-dialog');
						}
					});
					// build dialog content
					var html = '';
					var userName = usernameForUid(node.get('uid'));
					//html += '<p>Editing Event Request <em>"' + node.get('title') + '"</em> created by <em> ' + userName + '</em></p>';
					// notify coordinator if request was previously denied.
					if (this.isDenied()) {
						html += '<p class="approve-note">Note: This request was previously denied.  Approving this request will reset the denial.</p>';
					}
					// build form
					html += '<p>Select one of the three possible dates chosen by <em>' + userName + '</em>, or the suggested date if denied, then click Approve.</p>';
					if (accepted != undefined && this.isDenied()) {
						//var accepted_ts = datestampToDate(accepted.timestamp);
						var accepted_ts = accepted.timestamp_formatted;
						html += '<p class="approve-note">Note: The initiator has accepted the suggested date on ' + accepted_ts + '.  It has been selected as the default choice for approval below.</p>';
					}
					var $form = $('<form id="choose-date-form"><label for="choose-date">Possible Dates</label><select id="choose-date" name="choose-date">' + options + '</select></form>');
					if (requested_private) {
						$form.append('<div class="field field-field-request-private-event"><input type="checkbox" name="approve_private_request" id="approve-private-request" /><label for="approve_private_request">This event was requested to be private.  Check here to approve this event as private.</label></div>');
					}
					dialog.html(html);
					dialog.append($form);
					var $validationWrapper = $('<div id="dialog-errors" class="dialog-errors" style="display:none;" />');
					$form.prepend($validationWrapper);
					dialog.dialog('open');
				},
				approveEventRequest: function(date_value, approvedPrivate) {
					window.loadingIndicator.show();
					var node = this.model,
						view = this,
						dialog = appState.dialog;

					dialog.dialog('option', 'close', function() {
						var approve_private_bool = dialog.parent().find('[name="approve_private_request"]').length ? dialog.parent().find('[name="approve_private_request"]').is(':checked') : null;
						$.ajax({
							type: "POST",
							url: 'event_ajax/' + node.get('nid') + '/approve',
							data: {
								date: date_value,
								approve_private: approve_private_bool
							},
							success: function(data, textStatus, jqxhr) {
								var html = 'Event Request has been approved.';
								config.fns.launchMessageDialog(html, 'success', function() {
									window.loadingIndicator.hide();
									pollForContent();
								});
							},
							error: config.fns.handleError
						});
					});
					dialog.dialog('close');
					return false;
				},
				unapproveEventRequest: function(e) {
					e.preventDefault();
					window.loadingIndicator.show();
					var node = this.model;
					this.prepareToSave();
					// set unapproved
					node.save({
						field_request_status: {
							und: ["review"]
						},
						field_denied_on_date: {
							und: [{
								value: {
									date: null
								}
							}]
						},
						field_denied_suggested_date: {
							und: [{
								value: {
									date: null
								}
							}]
						},
						field_approved_date: {
							und: [null]
						}
					}, {
						wait: true,
						success: function(model, response, options) {
							//window.loadingIndicator.hide();
							var html = 'Event Request has been unapproved.';
							config.fns.launchMessageDialog(html, 'success');
						},
						error: config.fns.handleError
					});
					return false;
				},
				beginRequestDeny: function(e) {
					e.preventDefault();
					// store this NodeView in appState
					appState.lastView = this;


					var dates = this.getDates(),
						olddates = '<ul>';
					for (var x = 0, date; date = dates.und[x]; x++) {
						olddates += '<li>' + dateRange(date) + '</li>';
					}
					olddates += '</ul>';

					var dialog = appState.dialog;

					var $denyMarkup = $("<div />");
					$denyMarkup.append('<p>To deny an event request, you are confirming the following dates are not available for the initiator</p>' + olddates + '<p>Choose a new possible date for the event initiator, and click "Deny".</p>');
					//html += '<form id="deny-event-request-form"><label for="new-date">New Date</label><input type="text" name="new-date" /></form>';
					dialog.html($denyMarkup);
					// Append denial date field and date/time pickers
					var $denyForm = $("#suggested-denial-date-form").show().detach();
					var $denyMessage = $("#denial-message-form").show().detach();
					dialog.append($denyForm).append($denyMessage);

					var $denyValidationWrapper = $('<div id="dialog-errors" class="dialog-errors" style="display:none;" />');
					$denyForm.prepend($denyValidationWrapper);

					// Build out dialog
					dialog.dialog('option', {
						width: "auto",
						title: 'Event Denial',
						open: function() {
							//;                    $('#suggested-denial-date-form [name="field_denied_suggested_date[und][0][value][time]"]').timepicker(config.timeEntrySettings);
							//                  $('#suggested-denial-date-form [name="field_denied_suggested_date[und][0][value2][time]"]').timepicker(config.timeEntrySettings);
							dialog.parent().removeClass('approve-dialog').addClass('deny-dialog');
							var $display = dialog.parent().find('.timepicker-input-display');
							$display.each(function() {
								$(this).outerWidth($(this).prev().outerWidth() - 2);
								$(this).outerHeight($(this).prev().outerHeight() - 2);
								$(this).position({
									my: 'center center',
									at: 'center center',
									of: $(this).prev()
								});
							})
							dialog.parent().find('[name="field_denied_suggested_date[und][0][value][date]"]')
								.unbind()
								.once('adddatepicker')
								.datepicker(config.datepickerSettings)
								.click(function() {
									$(this).focus()
								}); // ie fix
							dialog.parent().find('[name="field_denied_suggested_date[und][0][value2][date]"]')
								.unbind()
								.once('adddatepicker')
								.datepicker(config.datepickerSettings)
								.click(function() {
									$(this).focus()
								}); // ie fix
						},
						close: function() {
							dialog.parent().removeClass('deny-dialog');
							// move denial date field back to app for re-use.
							$("#eaton-event-requests-app").append($("#suggested-denial-date-form").detach().hide());
							$("#eaton-event-requests-app").append($("#denial-message-form").detach().hide());
							//window.loadingIndicator.hide();
						},
						buttons: [{
							text: "Deny",
							click: function() {
								//var suggested_date = Date.parse($(this).find('form input[name="new-date"]').val()).toString('yyyy-MM-dd');
								var errors = [];
								$denyValidationWrapper.empty();
								var suggestion = {
									all_day: false,
									start: {
										date: $(this).find('[name="field_denied_suggested_date[und][0][value][date]"]').val()
									}
								}
								var $startTime = $(this).find('[name="field_denied_suggested_date[und][0][value][time]"]');
								var suggestion_message = $(this).find('[name="field_denied_message[und][0][value]"]').val() || '';
								if (!suggestion.all_day) {
									suggestion.start.time = $startTime.val();
									if (!suggestion.start.time.match(/^\d{1,}:\d{2}$/)) {
										errors.push('Invalid time entered for start time.');
									}
								}
								if (suggestion.start.date == '') {
									errors.push("The start date cannot be empty.");
								} else {
									var has_end = true;
									if (has_end) {
										suggestion.end = {
											date: $(this).find('[name="field_denied_suggested_date[und][0][value2][date]"]').val()
										}
										if (!suggestion.all_day) {
											suggestion.end.time = $(this).find('[name="field_denied_suggested_date[und][0][value2][time]"]').val();
											if (!suggestion.end.time.match(/^\d{1,}:\d{2}$/)) {
												errors.push('Invalid time entered for end time.');
											}
										}
									}
									if (has_end && suggestion.end.date == '') {
										errors.push("The end date cannot be empty.");
									} else {
										if (suggestion.all_day) {
											// no times
											suggestion.start.obj = Date.parse(suggestion.start.date);
											if (has_end) {
												suggestion.end.obj = Date.parse(suggestion.end.date);
											}
										} else {
											suggestion.start.obj = Date.parse(suggestion.start.date + ' ' + suggestion.start.time);
											if (suggestion.start.time == '') {
												errors.push("The start time field must be filled out if this is not an All Day event.");
											}

											if (has_end) {
												suggestion.end.obj = Date.parse(suggestion.end.date + ' ' + suggestion.end.time);
												if (suggestion.end.time == '') {
													errors.push("The end time field must be filled out if this is not an All Day event.");
												}
											}
										}

										if (has_end) {
											if (suggestion.end.obj != null && suggestion.end.obj.isBefore(suggestion.start.obj)) {
												errors.push("The end date cannot be before the start date.");
											}
										}
									}
								}


								if (errors.length) {
									var errorList = arrayToUL(errors);
									$denyValidationWrapper.append(errorList);
									$denyValidationWrapper.show("blind", 300);
								} else {
									// no errors
									$denyValidationWrapper.hide("blind", 300);
									appState.lastView.denyEventRequest(suggestion, suggestion_message);
								}
							}
						}, {
							text: "Cancel",
							click: function() {
								$(this).dialog('close');
							}
						}],
					});


					// Bind to denial date form
					$('#suggested-denial-date-form #edit-field-denied-suggested-date-und-0-show-todate').change(function() {
						if (this.checked) {
							$("#suggested-denial-date-form .end-date-wrapper").show().children().show();
							var $display = $("#suggested-denial-date-form .end-date-wrapper").find('.timepicker-input-display');
							$display.outerWidth($display.prev().outerWidth() - 2);
							$display.outerHeight($display.prev().outerHeight() - 2);
							$display.position({
								my: 'center center',
								at: 'center center',
								of: $display.prev()
							});
						} else {
							$("#suggested-denial-date-form .end-date-wrapper").hide();
						}
					});
					$('#suggested-denial-date-form #edit-field-denied-suggested-date-und-0-all-day').change(function() {
						if (this.checked) {
							$('#suggested-denial-date-form .start-date-wrapper .form-item-field-denied-suggested-date-und-0-value-time').hide();
							$('#suggested-denial-date-form .end-date-wrapper .form-item-field-denied-suggested-date-und-0-value2-time').hide();
						} else {
							$('#suggested-denial-date-form .start-date-wrapper .form-item-field-denied-suggested-date-und-0-value-time').show();
							$('#suggested-denial-date-form .end-date-wrapper .form-item-field-denied-suggested-date-und-0-value2-time').show();
							//     $('#suggested-denial-date-form [name="field_denied_suggested_date[und][0][value2][time]"]').timeEntry(config.timeEntrySettings);
						}
					});
					dialog.dialog('open');
				},
				denyEventRequest: function(suggestion, suggestion_message) {
					var dialog = appState.dialog;
					dialog.dialog('close');
					var node = this.model;

					var suggested_date = {
						'all_day': 0
					};
					if (suggestion.hasOwnProperty('start')) {
						suggested_date.value = {
							date: suggestion.start.date
						};
						if (suggestion.start.hasOwnProperty('time')) {
							suggested_date.value.time = suggestion.start.time;
						}
					}
					if (suggestion.hasOwnProperty('end')) {
						suggested_date.value2 = {
							date: suggestion.end.date
						};
						if (suggestion.end.hasOwnProperty('time')) {
							suggested_date.value2.time = suggestion.end.time;
						}
						suggested_date.show_todate = 1;
					}
					var tz_offset = new Date().getTimezoneOffset() / 60 * -1;
					$.ajax({
						type: "POST",
						url: 'event_ajax/' + node.get('nid') + '/deny',
						data: {
							suggested_date: suggested_date,
							suggested_message: suggestion_message,
							client_timezone: tz_offset
						},
						success: function(data, textStatus, jqxhr) {
							if (data.saved) {
								var denied_date_formatted = data.denied_date_formatted;
								var html = '<p>Event Request has been denied and a new date of ' + denied_date_formatted + ' has been suggested to the initiator.</p>';
								config.fns.launchMessageDialog(html, 'success', function() {
									window.loadingIndicator.hide();
									pollForContent();
								});
							} else {
								config.fns.handleError(node, jqxhr, {});
							}
						},
						error: config.fns.handleError
					});
				},
				deleteRequestConfirm: function(e) {
					if (e != undefined) {
						e.preventDefault();
						e.stopPropagation();
					}
					var title = this.model.get('title');
					appState.lastView = this;
					var opts = {
						'title': 'Are you sure?',
						width: "auto",
						buttons: [{
							"text": "Yes",
							click: function() {
								appState.lastView.deleteRequest();
							}
						}, {
							"text": "Cancel",
							click: function() {
								$(this).dialog('close');
							}
						}]
					};
					opts.open = function() {
						appState.dialog.parent().removeClass('approve-dialog message-dialog sucess-dialog').addClass('delete-dialog');
					};
					opts.close = function() {
						appState.dialog.parent().removeClass('delete-dialog');
					};
					var html = '<p>Are you sure you want to remove the Event Request <em>' + title + '</em>?  This cannot be undone.</p>';
					config.fns.launchDialog(opts, html);
				},
				deleteRequest: function() {
					var node = this.model,
						title = this.model.get('title'),
						dialog = appState.dialog;
					window.loadingIndicator.show();
					dialog.dialog('option', 'close', function() {

						node.destroy({
							'success': function() {
								window.loadingIndicator.hide();
								var html = '<p>Event Request <em>' + title + '</em> has been succesfully removed from the system.</p>';
								config.fns.launchMessageDialog(html, 'success');
							},
							'error': config.fns.handleError
						});
					});
					dialog.dialog('close');
				}
			});

			// # Appview
			//
			var AppView = Drupal.Backbone.View.extend({
				// Anchor our app on a prerendered div that comes from the app tpl.
				el: '#eaton-event-requests-app',

				initialize: function() {
					Drupal.Backbone.View.prototype.initialize.apply(this);
				
					NodeCollection.on('sort', function() {
						this.addAll();
						this.render();
					}, this);

					// Apply datepicker
					var filter_datepicker_settings = config.datepickerSettings;
					delete filter_datepicker_settings['minDate'];
					$('.filter-field .datepicker').datepicker(filter_datepicker_settings);

					// Begin polling
					pollForContent();
				},
				reset: function() {
					$('#eaton-event-requests-container tbody').html('');
					appState.showing = 0;
				},
				// Empty render function as we are tying our app to an existing div.
				render: function() {
					if (appState.showing > 0) {
						// add odd/even classes to table
						$("#eaton-event-requests-app table tbody").find('tr').once('apply-classes').each(function(k, v) {
							if (k % 2 == 0) {
								if ($(this).hasClass('odd')) {
									$(this).removeClass('odd');
								}
								$(this).addClass('even');
							} else {
								if ($(this).hasClass('even')) {
									$(this).removeClass('even');
								}
								$(this).addClass('odd');
							}
						});
						$('#emptyRow').remove();
					} else {
						var $table = $("#eaton-event-requests-app table");
						$table.children('tbody').html('<tr id="emptyRow"><td colspan="' + $table.find('th').length + '">Sorry, either no events exist or no events match your filter criteria.</td></tr>');
					}
				},
				// This gets triggered when an item is added the the NodeCollection.
				// We are passed the node model, so we want to create a NodeView from the
				// model, and add that to our table.
				addOne: function(node) {
					console.log('add node', node);
					var dates = node.get('field_possible_dates');
					if (dates.hasOwnProperty('und')) {
						for (var i = 0, d; d = dates.und[i]; i++) {
							if (d.timezone_db == 'UTC') {
								var start_date = Date.parse(d.value);
								var timezone_offset = start_date.getTimezoneOffset() / 60 * -1;
								d.value = start_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
								if (d.hasOwnProperty('value2') && d.value2 != null) {
									var end_date = Date.parse(d.value2);
									var timezone_offset = end_date.getTimezoneOffset() / 60 * -1;
									d.value2 = end_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
								}
								d.timezone_db = Drupal.settings.eaton_backbone.timezone_db;
							}
						}
						node.set('field_possible_dates', dates);
					}
					var approved_date = node.get('field_approved_date');

					var denied_date = node.get('field_denied_suggested_date');

					// Create a NodeView using the model passed in as our node.
					var view = new NodeView({
						model: node
					});
					// give unique ID
					view.$el.attr('id', 'event-request-' + node.id);
					// give classes
					if (view.isApproved()) {
						// approved
						view.$el.addClass('approved');
					} else if (view.isDenied()) {
						view.$el.addClass('denied');
					}

					if (view.isPrivate()) {
						view.$el.addClass('private');
					}
					// Render the view element and add it to the table.
					$('#eaton-event-requests-container tbody').append(view.render().el);
				},
				addAll: function() {
					window.loadingIndicator.show();
					if (!appState.touched_location) {
						if (Drupal.settings.event_requests.default_location != undefined && Drupal.settings.event_requests.default_location != '') {
							$("#filter-field_event_location", context).val(Drupal.settings.event_requests.default_location).trigger('chosen:updated')
							appState.touched_location = true;
						}
					}
					if (!appState.touched_mindate) {
						if ($("#filter-field_possible_dates-min").val() == "") {
							$("#filter-field_possible_dates-min").val(Date.today().toString('yyyy-MM-dd'))
						}
						appState.touched_mindate = true;
					}
					var injectFilterValues = ['field_possible_dates-min', 'field_possible_dates-max', 'field_event_location', 'field_event_visibility', 'field_request_status'];

					for (var filterName, i = 0; filterName = injectFilterValues[i]; i++) {
						var $filter = $("#filter-" + filterName);

						if ($filter.length) {
							var found = false;
							if (appState.filter.length) {
								var find = appState.filter.filter(function(obj, idx) {
									return obj.field === filterName;
								});
								if (find.length) {
									found = true;
									var idx = appState.filter.indexOf(find[0]);
									appState.filter[idx].val = $filter.val();
								}
							}

							if (!found && $filter.val() != -1) {
								appState.filter.push({
									'field': filterName,
									'val': $filter.val()
								});
							}
						}
					}

					if (!NodeCollection.models.length) {
						var $table = $("#eaton-event-requests-app table");
						$table.children('tbody').html('<tr><td colspan="' + $table.find('th').length + '">Sorry, either no events exist or no events match your filter criteria.</td></tr>');
					} else {
						this.reset();
						appState.showing = 0;
						if (!appState.filter.length) {
							NodeCollection.fetch({update:true});
						} else {
							// chain filters together.
							var filtered = NodeCollection.filter(function(model) {
								var matches = [];
								for (var f, k = 0; f = appState.filter[k]; k++) {
									if (f.field == 'field_possible_dates-min') {
										if (f.val == "") {
											matches[k] = true;
										} else {
											var dates = model.get('field_possible_dates');
											var match = false;
											var filterDate = Date.parse(f.val);
											var approved_date = model.get('field_approved_date');
											if (approved_date.hasOwnProperty('und')) {
												var date1 = Date.parse(approved_date.und[0].value),
													date2 = Date.parse(approved_date.und[0].value2);

												match = date1.isAfter(filterDate) || (date1.isBefore(filterDate) && date2.isAfter(filterDate));
											}
											if (!match) {
												var findDates = dates.und.filter(function(date) {
													var date1 = Date.parse(date.value.split(' ')[0]),
														date2 = Date.parse(date.value2.split(' ')[0]);
													return filterDate == null || Date.equals(filterDate, date1) || Date.equals(filterDate, date2) || 
														(date1.isBefore(filterDate) && date2.isAfter(filterDate)) || date1.isAfter(filterDate);
												});

												if (findDates.length) {
													match = true;
												}
											}

											matches[k] = match;
										}
									} else if (f.field == 'field_possible_dates-max') {
										if (f.val == "") {
											matches[k] = true;
										} else {
											var dates = model.get('field_possible_dates');
											var match = false;
											var filterDate = Date.parse(f.val).clearTime();
											var approved_date = model.get('field_approved_date');
											if (approved_date.hasOwnProperty('und')) {
												var date1 = Date.parse(approved_date.und[0].value),
													date2 = Date.parse(approved_date.und[0].value2);

												match = date1.isBefore(filterDate) || date2.isBefore(filterDate);
											}
											if (!match) {
												var findDates = dates.und.filter(function(date) {
													var date1 = Date.parse(date.value.split(' ')[0]),
														date2 = Date.parse(date.value2.split(' ')[0]);
													return filterDate == null || Date.equals(filterDate, date1) || Date.equals(filterDate, date2) || 
														(date1.isBefore(filterDate) && date2.isAfter(filterDate));
												});

												if (findDates.length) {
													match = true;
												}
											}

											matches[k] = match;
										}
									} else {
										var thisVal = model.get(f.field);
										if (typeof(thisVal.und) != "undefined") {
											thisVal = thisVal.und[0].value;
										}
										matches[k] = thisVal === f.val;
									}
								}
								return matches.reduce(function(previousVal, currentValue, currentIndex) {
									return previousVal && !!currentValue;
								}, true);
							});
							NodeCollection.reset(filtered, {
								silent: true
							});
						}

						if (((appState.page * config.itemsPerPage) - config.itemsPerPage) > NodeCollection.models.length) {
							// go back a page if needed
							this.prevPage();
						}

						var x = 0;
						// add each node to the table if its on the current page (based on items per page, number of items, and current page)
						NodeCollection.each(function(model) {
							if (x >= ((appState.page - 1) * config.itemsPerPage) && appState.showing < config.itemsPerPage) {
								this.addOne(model);
								appState.showing++;
							}
							x++;
						}, this);
					}

					// update pager
					this.updatePager();
				},
				updatePager: function() {
					// update total # of nodes
					$("#max-count").html(NodeCollection.models.length);
					var display_min = -1,
						display_max = 0;
					if (NodeCollection.models.length > 0) {
						display_min = (appState.page - 1) * config.itemsPerPage;
						display_max = display_min + appState.showing;
					}

					$("#display-min").html(display_min + 1);
					$("#display-max").html(display_max);

					// clear pager html
					$("#eaton-event-requests-app-pager").html('');

					// add next pager button, if necessary
					if (appState.page > 1) {
						// add previous page
						$("#eaton-event-requests-app-pager").append('<a href="#" title="Previous Page" id="eaton-event-requests-previous">Previous</a>');
						$("#eaton-event-requests-previous").click(this.prevPage);
					}
					if (display_max < NodeCollection.length) {
						// add next page
						$("#eaton-event-requests-app-pager").append('<a href="#" title="Next Page" id="eaton-event-requests-next">Next</a>');
						$("#eaton-event-requests-next").click(this.nextPage);
					}
					$("time.timeago").timeago();
					$("#loadingIndicator").fadeOut(250);
				},
				prevPage: function(e) {
					if (typeof(e) != "undefined") {
						e.preventDefault();
					}
					window.loadingIndicator.show();
					appState.page--;
					app.addAll();
				},
				nextPage: function(e) {
					if (typeof(e) != "undefined") {
						e.preventDefault();
					}
					window.loadingIndicator.show();
					appState.page++;
					app.addAll();
				}
			});

			var pollForContent = function() {
				var refreshRate = $(".table-refresh-container select").val();
				$.cookie('eaton_backbone_table_refresh_rate', refreshRate);
				if (!appState.dialog.dialog('isOpen')) {
					NodeCollection.fetch({
						update: true
					});
				}
				clearTimeout(pollTimer);
				if (refreshRate > 0) {
					pollTimer = setTimeout(pollForContent, refreshRate);
				}
			};

			// ### Start the app!
			var app = new AppView();

			// Sort event triggers
			var lastclicked = $('<div />');
			$("#eaton-event-requests-app th a").click(function(e) {
				e.preventDefault();
				if (lastclicked.is($(this))) {
					// swap sort order
					NodeCollection.sortOrder = NodeCollection.sortOrder == 'asc' ? 'desc' : 'asc';
				}
				lastclicked = $(this);
				var key = $(this).attr('data-key');
				NodeCollection.sortByField(key);
				$(this).parents('table').find('th').removeClass('sorting asc desc');
				$(this).parent().addClass('sorting ' + NodeCollection.sortOrder);
			});

			// Add options to filters, gather from .eaton-app-filters div
			var filters = [];
			var $filters = $('.eaton-app-filters .filter-field').each(function() {
				var $input = $(this).find('input');
				if ($input.length) {
					$input.each(function() {
						filters.push($(this).attr('name').replace('filter-', ''));
					})
				}
				var $select = $(this).find('select');
				if ($select.length) {
					$select.each(function() {
						filters.push($(this).attr('name').replace('filter-', ''));
					})
				}
			});
			for (var n = 0, f; f = filters[n]; n++) {
				// Add default options to SELECTs
				if ($("#filter-" + f)[0].tagName == 'SELECT') {
					var options = '<option value="-1">-- Any --</option>';
					for (var key in Drupal.settings.event_requests.options[f]) {
						options += '<option value="' + key + '">' + Drupal.settings.event_requests.options[f][key] + '</option>';
					}
					$("#filter-" + f).html(options);
				}
				$("#filter-" + f).change(function() {
					window.loadingIndicator.show();
					if (hasStorage) {
						var itemKey = 'eaton_backbone_event_requests_filter_' + $(this).attr('id').replace('filter-', '');
						var itemVal = $(this).val()
						localStorage.setItem(itemKey, itemVal);
					}
					NodeCollection.filterBy($(this).attr('id').replace('filter-', ''), $(this).val());
				});
				if (hasStorage) {
					var storedValue = localStorage.getItem('eaton_backbone_event_requests_filter_' + f)
					if (storedValue != null) {
						$("#filter-" + f).val(storedValue);
					}
				}
			}

			// bind items per page
			$(".eaton-app-items-per-page").change(function() {
				var old_items_per_page = config.itemsPerPage;
				config.itemsPerPage = parseInt($(this).val());
				while (((appState.page - 1) * config.itemsPerPage) > NodeCollection.length) {
					// if the new range on the current page is larger than the collection, go back a page until we have a range to show.
					appState.page--;
				}
				app.addAll();
			});

			$(document).ready(function() {
				// Apply chosen
				//$(".filter-field select,.eaton-app-items-per-page").chosen({disable_search:true});
				$(".filter-field select").chosen({
					disable_search: true
				});
			});



			// private functions
			var usernameForUid = function usernameForUid(uid) {
				return Drupal.settings.event_requests.users[uid].name || 'user ' + uid;
			}
			var fullnameForUid = function fullnameForUid(uid) {
				return Drupal.settings.event_requests.users[uid].field_first_name_value + Drupal.settings.event_requests.users[uid].field_last_name_value || 'user ' + uid;
			}
			var arrayToUL = function arrayToUL(arr) {
				var ul = '<ul>';
				for (var i = 0, x; x = arr[i]; i++) {
					ul += '<li>' + x + '</li>';
				}
				ul += '</ul>';
				return ul;
			}
		},

		// ## unattach()
		//
		// Just to be consistent with Drupal standards, we provide an unattach
		// function as well.
		unattach: function() {
			$('#eaton-event-requests-app').html('');
		}
	};
})(jQuery);

// Formatting functions called from the template, need to be global.

/**
 * Add Reforms htmlEncode function to our window for use in underscore templates
 * and in our js code.
 * @see https://code.google.com/p/reform/
 */
window.htmlEncode = function HtmlEncode($str, $default) {
	if ($str == null || $str.length == 0) {
		$str = ($default == null ? '' : $default);
	}

	$out = '';
	$len = $str.length;

	// Allow: a-z A-Z 0-9 SPACE , .
	// Allow (dec): 97-122 65-90 48-57 32 44 46

	for ($cnt = 0; $cnt < $len; $cnt++) {
		$c = $str.charCodeAt($cnt);
		if (($c >= 97 && $c <= 122) ||
			($c >= 65 && $c <= 90) ||
			($c >= 48 && $c <= 57) ||
			$c == 32 || $c == 44 || $c == 46) {
			$out += $str.charAt($cnt);
		} else {
			$out += '&#' + $c + ';';
		}
	}

	return $out;
}

window.valueForKey = function(field, key) {
	return Drupal.settings.event_requests.options[field][key];
}