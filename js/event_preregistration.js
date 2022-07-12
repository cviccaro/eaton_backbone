/**
 * This file acts on event preregistration page to handle
 * attendees table, editing attendees, etc.
 */

(function($) {
	Drupal.behaviors.event_preregistration = {
		attach: function(context, settings) {
			if (typeof(settings.event_preregistration) != "undefined") {

				var $form = $("#eaton-backbone-event-registrant-form", context); // Add Registrant form
				var $editForm = $("#eaton-backbone-event-registrant-edit-form", context); // Edit Registrant form
				var $editFormValidation = $("#clientsidevalidation-eaton-backbone-event-registrant-edit-form-errors", context); // Hijack the edit form's clientside validation wrapper
				var $table = $("#eaton-event-attendees-container"); //Event attendees table, running backbone here

				var node = settings.event_preregistration.node;
				var NodeModel = new Drupal.Backbone.Models.Node(node); // place node in backbone model for ease-of-use

				var picture_id = '';

				// Configuration
				var config = {
					restEndpoint: '/eaton_backbone/rest',
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
								height: "auto",
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
							// set new options
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
									//console.log('adding class to message-dialog: ' + type + '-dialog');
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
					}
				};
				// Create object to hold application state
				var appState = {
					showing: 0,
					interval: 0,
					page: 1,
					filter: [],
					dialog: {},
					lastView: null
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
					buttons: config.defaultDialogButtons,
					close: function() {
						startPolling();
					},
					open: function() {
						stopPolling();
					}
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
				$("#eaton-event-attendees-items-per-page").html(itemsPerPageOptions);

				/*
				 * CSRF fix from https://groups.drupal.org/node/358308
				 */
				$.ajax({
					url: "/services/session/token",
					type: "get",
					dataType: "text",
					error: function(jqXHR, textStatus, errorThrown) {
						// alert(errorThrown);
					},
					success: function(token) {
						$.ajaxSetup({
							beforeSend: function(request) {
								request.setRequestHeader("X-CSRF-Token", token);
							}
						});
					}
				});

				// Declare Backbone Model, Collection, View
				var AttendeeModel = Drupal.Backbone.Models.Attendee; // from attendeeresource-services.js
				var AttendeeCollection = new Drupal.Backbone.Collections.EventAttendeesView({
					eventId: node.nid,
					sortKey: 'timestamp'
				}); // from attendeeresource-services.js

				var AttendeeView = Drupal.Backbone.View.extend({
					templateSelector: '#eaton-backbone-event-attendees-template',
					tagName: 'tr',
					initialize: function(opts) {
						this.listenTo(this.model, 'change', this.render);
						this.listenTo(this.model, 'destroy', this.remove);
						Drupal.Backbone.View.prototype.initialize.apply(this);
					},
					render: function() {
						appState.lastView = this;
						Drupal.Backbone.View.prototype.render.call(this);
						// Update badge links
						this.$el.find('.badge-button').attr('href', window.location.protocol + '//' + window.location.hostname + '/badge/' + node.nid + '/' + this.model.get('attendee_id')).attr('target', '_blank');
						this.$el.find('.badge-button-back').attr('href', window.location.protocol + '//' + window.location.hostname + '/badge/' + node.nid + '/' + this.model.get('attendee_id') + '/back').attr('target', '_blank');
						return this;
					},
					events: {
						'click .edit-button': 'editAttendee',
						'click .remove-button': 'removeAttendeeConfirm',
						'click .email-button': 'dispatchAttendeeEmailConfirm',
						'click .check-inout-button': 'checkInOutConfirm',
					},
					fillInEditForm: function($editForm) {
						var attendee = this.model;
						var $emptyPicture = $("#event_registrant_current_picture").siblings('.empty'); // no-picture text
						var $removePictureButton = $("#event_registrant_current_picture").siblings('button');
						var $currentPicture = $("#event_registrant_current_picture");

						// By default, assume there is no picture
						$emptyPicture.show();
						$removePictureButton.hide();
						$currentPicture.attr('src', '');

						for (var key in attendee.attributes) {
							var val = attendee.get(key);
							if (val != null) {
								var $field = $('[name="' + key + '"]', $editForm);
								if ($field.length) {
									// If field is a file..
									if ($field.prop('tagName') == 'INPUT' && $field.attr('type') == 'file') {
										// get picture through REST Services
										$emptyPicture.text('Photo found.  Loading...');
										$.get(config.restEndpoint + '/file/' + val, function(data, textStatus, xhr) {
											if (textStatus == 'success') {
												var file = data;
												// hide empty text
												$emptyPicture.hide();
												// show remove button
												$removePictureButton.show();
												// place image url in src
												$currentPicture.attr('src', file.uri_full);
												$currentPicture.show();
												// bind to remove button
												$removePictureButton.click(function(e) {
													e.preventDefault();
													e.stopPropagation();

													$removePictureButton.hide();
													$emptyPicture.text('None').show();
													$currentPicture.hide().attr('src', '');

													// Store old picture as attribute to restore if needed
													attendee.set('old_picture', attendee.get('picture'));
													attendee.set('picture', null);
												});
											}
										}, "json");
									} else {
										// Else just fill in the field value
										//console.log('setting ' + $field.attr('name') + ' to ' + val);
										$field.val(val);
									}
								}
							} else {
								// handle null
								if (key == 'picture') {
									$emptyPicture.text('None');
									$currentPicture.hide();
								}
							}
						}
						// Bind to edit form's picture file element
						$editForm.find('input[type="file"]').unbind().bind('change', function(e) {
							//console.log(e);
							var files = e.target.files;
							if (files.length) {
								if (files[0].type == 'image/gif' || files[0].type == 'image/png' || files[0].type == 'image/jpg' || files[0].type == 'image/jpeg') {
									var reader = new FileReader();
									reader.onload = function(readerEvt) {
										var binaryString = readerEvt.target.result;
										$("#event_registrant_new_picture").attr('src', 'data:image/png;base64,' + btoa(binaryString)).parent().show();
										$("#event_registrant_new_picture").attr('data-filename', files[0].name);
									}
									reader.readAsBinaryString(files[0]);
								} else {
									$("#clientsidevalidation-eaton-backbone-event-registrant-edit-form-errors", context).show();
									$("#clientsidevalidation-eaton-backbone-event-registrant-edit-form-errors ul", context).append('<li>Picture must be an image file of type gif, jpg, or png</li>').show();
									$(this).val('');
								}
							}
						})

					},
					editAttendee: function(e) {
						if (e != undefined) {
							e.preventDefault();
							e.stopPropagation();
						}
						var attendee = this.model;
						//console.log('editing',attendee);
						$editForm.find('.form-actions').remove(); // remove drupal form actions

						// Hide and prepare "upload picture" preview for re-use
						$editForm.find('.form-item-picture-preview').hide();
						$editForm.find('.form-item-picture-preview img').attr('src', '');
						$editForm.find('.form-item-picture-preview img')[0].removeAttribute('data-filename');
						$editForm.find('.form-item-picture input').val('');

						// Clear validation
						$editFormValidation.find('ul').empty();
						$editFormValidation.hide();


						var dialog = appState.dialog;
						var currentView = this;
						dialog.dialog('option', {
							minWidth: 700,
							height: "auto",
							resizable: true,
							open: function() {
								dialog.parent().addClass('edit-dialog');
							},
							close: function() {
								dialog.parent().removeClass('success-dialog error-dialog edit-dialog');
							},
							title: 'Editing Attendee ' + attendee.getLabel(),
							buttons: [{
								text: "Submit Changes",
								click: function() {
									window.loadingIndicator.show();
									// Check if there is a picture first.
									if ($editForm.find('#event_registrant_new_picture').attr('src') != '') {
										// If there is a picture, upload it first.  it gets validated first, so no bad values should hit this logic path.
										var $newPic = $editForm.find('#event_registrant_new_picture');
										var newPicData = {
											"file": {
												"file": $newPic.attr('src').replace('data:image/png;base64,', ''),
												"filename": $newPic.attr('data-filename'),
												"filepath": 'public://attendee_pictures/' + $newPic.attr('data-filename')
											}
										};
										$.ajax({
											type: "POST",
											data: newPicData,
											url: config.restEndpoint + '/file.json',
											dataType: 'json',
											success: function(result, textStatus, xhr) {
												if (textStatus == 'success') {
													// Store file_id as picture attribute, then submit the form
													picture_id = $.trim(result.fid);
													attendee.attributes.picture = picture_id;
													attendee.set('picture', picture_id);
													currentView.editAttendeeSubmit();
												}
											},
											error: config.fns.handleError,
											async: false
										});
									} else {
										// Submit form
										currentView.editAttendeeSubmit();
									}
								}
							}, {
								text: "Cancel",
								click: function() {
									// Restore old picture if needed
									if (attendee.get('old_picture') != undefined) {
										attendee.set('picture', attendee.get('old_picture'));
										attendee.unset('old_picture');
									}
									$(this).dialog('close');
									// Start polling again
									startPolling();
								}
							}]
						});
						// append edit form
						dialog.empty().append($editForm.show());
						// append its validation wrapper
						dialog.prepend($("#clientsidevalidation-eaton-backbone-event-registrant-edit-form-errors", context));
						// fill in form with values
						this.fillInEditForm($editForm);
						dialog.dialog('open');
						// apply chosen
						//                        $editForm.find('select').each(function() {
						//                            if (!$(this).is(':visible')) { 
						//                                var id = $(this).attr('id').replace('-','_').replace('--','__');
						//                                $(this).siblings('#' + id + '_chosen').remove();
						//                                $(this).show();
						//                            }
						//                            $(this).chosen({disable_search:true}).trigger('chosen:updated');
						//                        });
						$editForm.find('select[name="country"]').change(); // trigger change
					},
					editAttendeeSubmit: function() {
						var attendee = this.model;
						var errors = [];
						// Collect input
						$editForm.find('.form-item').each(function() {
							var $field = false;
							if ($(this).hasClass('form-type-textfield')) {
								$field = $(this).find('input');
							} else if ($(this).hasClass('form-type-select')) {
								$field = $(this).find('select');
							}
							if ($field) {
								var val = $field.val();
								if (val.length) {
									attendee.set($field.attr('name'), val);
								} else {
									errors.push($field.attr('name'));
								}
							}
						});
						var country = $editForm.find('[name="country"]').val();
						var state = $editForm.find('[name="state"]').val();
						if (Drupal.settings.country_taxonomy.countries[country].hasOwnProperty('states') && (state == -1 || state == '')) {
							errors.push('state');
						}
						// Validate
						if (!errors.length) {
							appState.dialog.dialog('close');
							attendee.set('attendee_id', parseInt(attendee.get('attendee_id')));
							//console.log(attendee);
							attendee.save(null, {
								wait: true,
								success: function(model, response, options) {
									window.loadingIndicator.hide();
									var html = 'Attendee ' + attendee.getLabel() + ' has been successfully updated.';
									config.fns.launchMessageDialog(html, 'success', function() {
										pollForContent();
									});
								},
								error: config.fns.handleError
							});
						} else {
							// show errors
							window.loadingIndicator.hide();
							var error_list = '';
							for (var x = 0, field_name; field_name = errors[x]; x++) {
								var msg = field_name.replace('_', ' ').capitalize() + ' cannot be left blank.';
								if (field_name == 'state') {
									msg = 'State cannot be left blank when country chosen is ' + Drupal.settings.country_taxonomy.countries[country].name;
								}
								error_list += '<li>' + msg + '</li>';
							}
							$editFormValidation = $("#clientsidevalidation-eaton-backbone-event-registrant-edit-form-errors");
							$editFormValidation.show();
							$editFormValidation.append(error_list).show();
						}
					},
					removeAttendeeConfirm: function(e) {
						appState.lastView = this;
						var attendee = this.model;
						if (typeof(e) != "undefined") {
							e.preventDefault();
							e.stopPropagation();
						}
						var dialog = appState.dialog;
						var opts = {
							title: 'Are you sure?',
							buttons: [{
								text: "Yes",
								click: function() {
									$(this).dialog('close');
									appState.lastView.removeAttendee();
								}
							}, {
								text: "Cancel",
								click: function() {
									$(this).dialog('close');
								}
							}]
						};
						var html = '<p>Are you sure you want to remove ' + attendee.getLabel() + ' from this event?</p>';
						config.fns.launchDialog(opts, html);
					},
					removeAttendee: function(e) {
						if (e != undefined) {
							e.preventDefault();
							e.stopPropagation();
						}
						var attendee = this.model;
						window.loadingIndicator.show();
						$.ajax({
							url: config.restEndpoint + "/attendee/unregister/" + node.nid + '/' + attendee.get('attendee_id'),
							type: "post",
							dataType: "json",
							error: config.fns.handleError,
							success: function(data, textStatus, jqXHR) {
								window.loadingIndicator.hide();
								if (data && typeof(data.status) != "undefined") {
									var dialog = appState.dialog;
									if (data.status == 'success') {
										AttendeeCollection.reset().fetch({
											update: true
										});
										var html = 'Attendee ' + attendee.getLabel() + ' has been successfully removed from this event.';
										config.fns.launchMessageDialog(html, 'success');

									} else {
										var title = typeof(data.title) != "undefined" ? data.title : 'Something went wrong.';
										var html = title + '  ' + data.message;
										config.fns.launchMessageDialog(html, 'warning');
									}
								} else {

								}
							}
						});
					},
					dispatchAttendeeEmailConfirm: function(e) {
						if (e != undefined) {
							e.preventDefault();
							e.stopPropagation();
						}
						appState.lastView = this;
						var attendee = this.model;

						var dialog = appState.dialog;
						var opts = {
							title: 'Are you sure?',
							buttons: [{
								text: "Yes",
								click: function() {
									$(this).dialog('close');
									appState.lastView.dispatchAttendeeEmail();
								}
							}, {
								text: "Cancel",
								click: function() {
									$(this).dialog('close');
								}
							}],
							width: "auto"
						};
						var html = '<p>Are you sure you want to send a preregistration email to ' + attendee.getLabel() + '?</p>';
						config.fns.launchDialog(opts, html);
					},
					dispatchAttendeeEmail: function(e) {
						var attendee = this.model;
						window.loadingIndicator.show();
						attendee.dispatchEmail(config.restEndpoint, node.nid, function() {
							window.loadingIndicator.hide();
							var html = 'A preregistration email has been successfully sent to attendee <em>' + attendee.getLabel() + '</em>';
							config.fns.launchMessageDialog(html, 'success');
						}, config.fns.handleError);
					},
					checkInOutConfirm: function(e) {
						appState.lastView = this;
						var target = e.currentTarget,
							method = $(target).hasClass('check-in') ? 'in' : 'out',
							attendee = this.model,
							dialog = appState.dialog,
							opts = {
								title: 'Check ' + method + ' attendee ' + attendee.getLabel(),
								buttons: [{
									text: "Check " + method.capitalize(),
									click: function() {
										$(this).dialog('close');
										appState.lastView.checkInOut(attendee, method);
									},
                                'class': 'check-' + method
								}, {
									text: "Cancel",
									click: function() {
										$(this).dialog('close');
									}
								}],
								width: "auto"
							};
						var html = '<p>Are you sure you want to check ' + method + ' ' + attendee.getLabel() + '?</p>';
						config.fns.launchDialog(opts, html);
					},
					checkInOut: function(attendee, method, showDialogs) {
						if (showDialogs === undefined || showDialogs === null) {
							showDialogs = true;
						}
						//console.log(attendee, method);
						window.loadingIndicator.show();
						if (method == 'in') {
							attendee.checkIn(config.restEndpoint, node.nid, function() {
								window.loadingIndicator.hide();
								if (showDialogs) {
									var html = 'Attendee <em>' + attendee.getLabel() + '</em> has been successfully checked into this event.';
									config.fns.launchMessageDialog(html, 'check-in');
								}
								AttendeeCollection.fetch();
							}, config.fns.handleError);
						} else if (method == 'out') {
							attendee.checkOut(config.restEndpoint, node.nid, function() {
								window.loadingIndicator.hide();
								if (showDialogs) {
									var html = 'Attendee <em>' + attendee.getLabel() + '</em> has been successfully checked out of this event.';
									config.fns.launchMessageDialog(html, 'check-out');
								}
								AttendeeCollection.fetch();
							}, config.fns.handleError);
						}
					},
				});

				var AppView = Drupal.Backbone.View.extend({
					templateSelector: '#eaton-event-attendees-app',
					initialize: function() {
						Drupal.Backbone.View.prototype.initialize.apply(this);
						this.reset();
						//this.listenTo(AttendeeCollection, 'add', this.addOne);
						this.listenTo(AttendeeCollection, 'remove', this.removeOne);
						this.listenTo(AttendeeCollection, 'reset', this.addAll);
						this.listenTo(AttendeeCollection, 'all', this.render);
						AttendeeCollection.on('sort', this.addAll, this);
						startPolling();
						pollForContent();
					},
					reset: function() {
						appState.showing = 0;
						$("tbody tr", $table).each(function() {
							if (!$(this).is($("#empty-row", $table))) {
								$(this).remove();
							}
						});
					},
					render: function() {
						if (AttendeeCollection.models.length > 0) {
							$("#empty-row", $table).hide();
						} else {
							$("#empty-row", $table).show();
							$("#view_all_badges--front").show();
							$("#view_all_badges--back").show();
							//$("#send_all_emails").hide();
						}
					},
					addOne: function(attendee) {
						// Create a View
						//console.log(attendee);
						var view = new AttendeeView({
							model: attendee
						});
						// Render the view element and append it to the table
						$('tbody', $table).append(view.render().el);
					},
					addAll: function() {
						this.reset();
						var x = 0;
						appState.showing = 0;
						// add each node to the table if its on the current page (based on items per page, number of items, and current page)
						AttendeeCollection.each(function(model) {
							if (x >= ((appState.page - 1) * config.itemsPerPage) && appState.showing < config.itemsPerPage) {
								this.addOne(model);
								appState.showing++;
							}
							x++;
						}, this);
						this.updatePager();
						$(".eaton-app-table tbody tr").each(function(k, v) {
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
						if (appState.showing > 0) {
							$("#view_all_badges--front").show();
							$("#view_all_badges--back").show();
						}
					},
					removeOne: function(attendee) {
						//console.log('removeOne',attendee);
					},
					updatePager: function() {
						// update total # of nodes
						$("#max-count").html(AttendeeCollection.length);
						var display_min = -1,
							display_max = 0;
						if (AttendeeCollection.length > 0) {
							display_min = (appState.page - 1) * config.itemsPerPage;
							display_max = display_min + appState.showing;
						}

						$("#display-min").html(display_min + 1);
						$("#display-max").html(display_max);

						// clear pager html
						$(".eaton-app-pager").html('');

						// add next pager button, if necessary
						if (appState.page > 1) {
							// add previous page
							$(".eaton-app-pager").append('<a href="#" title="Previous Page" class="eaton-app-pager-previous">Previous</a>');
							$(".eaton-app-pager-previous").click(this.prevPage);
						}
						if (display_max < AttendeeCollection.length) {
							// add next page
							$(".eaton-app-pager").append('<a href="#" title="Next Page" class="eaton-app-pager-next">Next</a>');
							$(".eaton-app-pager-next").click(this.nextPage);
						}
						window.loadingIndicator.hide();
					},
					prevPage: function(e) {
						if (typeof(e) != "undefined") {
							e.preventDefault();
						}
						// reduce page number by 1, reset collection
						appState.page--;
						app.addAll();
					},
					nextPage: function(e) {
						e.preventDefault();
						// increase page number by 1, reset collection
						appState.page++;
						app.addAll();
					}
				});
				var pollTimer;

				function startPolling() {
					//  pollTimer = setTimeout(pollForContent,60000);
				}

				function pollForContent() {
					AttendeeCollection.fetch({
						update: true
					});
				}

				function stopPolling() {
					//        clearTimeout(pollTimer);
				}
				var app = new AppView();

				// Bind to table row headers for sorting
				$(".eaton-app th a").click(function(e) {
					e.preventDefault();
					if ($(this).parent().hasClass('sorting')) {
						// swap sort order
						AttendeeCollection.sortOrder = AttendeeCollection.sortOrder == 'asc' ? 'desc' : 'asc';
					}
					var key = $(this).attr('data-key');
					AttendeeCollection.sortByField(key);
					$(this).parents('table').find('th').removeClass('sorting asc desc');
					$(this).parent().addClass('sorting ' + AttendeeCollection.sortOrder);
				});

				// bind items per page
				$(".eaton-app-items-per-page").change(function() {
					var old_items_per_page = config.itemsPerPage;
					config.itemsPerPage = parseInt($(this).val());
					while (((appState.page - 1) * config.itemsPerPage) > AttendeeCollection.length) {
						// if the new range on the current page is larger than the collection, go back a page until we have a range to show.
						appState.page--;
					}
					app.addAll();
				});

				$("#send_all_emails").click(function(e) {
					e.preventDefault();
					e.stopPropagation();
					if (AttendeeCollection.models.length) {
						var emails = AttendeeCollection.map(function(model) {
							return '"' + model.get('first_name') + ' ' + model.get('last_name') + '" <' + model.get('email') + '>';
						});
						window.location = 'mailto:' + emails.join(',');
						// var dialog = appState.dialog;
						// var opts = {
						// 	title: 'Are you sure?',
						// 	buttons: [{
						// 		text: "Yes",
						// 		click: function() {
						// 			dialog.dialog('option', 'close', function() {
						// 				window.loadingIndicator.show();
						// 				AttendeeCollection.each(function(model) {
						// 					model.dispatchEmail(config.restEndpoint, node.nid);
						// 				});
						// 				window.loadingIndicator.hide();
						// 				config.fns.launchMessageDialog('Emails have been sent succesfully to all attendees.', 'success');
						// 			});
						// 			dialog.dialog('close');
						// 		}
						// 	}, {
						// 		text: "Cancel",
						// 		click: function() {
						// 			$(this).dialog('close');
						// 		}
						// 	}],
						// 	width: "auto"
						// };
						// var html = '<p>Are you sure you want to send all preregistration emails to this events\' attendees?</p>';
						// config.fns.launchDialog(opts, html);
					} else {
						config.fns.launchMessageDialog('No attendees have been registered for this event, so no emails can be sent.', 'info');
					}
				});

				$("#check_in_all").click(function(e) {
					if (e) {
						e.preventDefault();
						e.stopPropagation();
					}
					var dialog = appState.dialog;
					var opts = {
						title: 'Are you sure?',
						buttons: [{
							text: "Yes",
							click: function() {
								dialog.dialog('option', 'close', function() {
									AttendeeCollection.each(function(model) {
										appState.lastView.checkInOut(model, 'in', false);
									});
									config.fns.launchMessageDialog('All attendees have been checked in.', 'success');
								});
								dialog.dialog('close');
							}
						}, {
							text: "Cancel",
							click: function() {
								$(this).dialog('close');
							}
						}],
						width: "auto"
					};
					var html = '<p>Are you sure you want to check in all of this event\'s attendees?</p>';
					config.fns.launchDialog(opts, html);
				});

				$("#check_out_all").click(function(e) {
					if (e) {
						e.preventDefault();
						e.stopPropagation();
					}
					var dialog = appState.dialog;
					var opts = {
						title: 'Are you sure?',
						buttons: [{
							text: "Yes",
							click: function() {
								dialog.dialog('option', 'close', function() {
									AttendeeCollection.each(function(model) {
										appState.lastView.checkInOut(model, 'out', false);
									});
									config.fns.launchMessageDialog('All attendees have been checked out.', 'success');
								});
								dialog.dialog('close');
							}
						}, {
							text: "Cancel",
							click: function() {
								$(this).dialog('close');
							}
						}],
						width: "auto"
					};
					var html = '<p>Are you sure you want to check out all of this event\'s attendees?</p>';
					config.fns.launchDialog(opts, html);
				});

				$("#rebuild_badges").change(function() {
					if ($(this).is(':checked')) {
						var href = $("#view_all_badges--front").attr('href');
						$("#view_all_badges--front").attr('href', href + '?rebuild=1');

						href = $("#view_all_badges--back").attr('href');
						$("#view_all_badges--back").attr('href', href + '?rebuild=1');
					} else {
						var href = $("#view_all_badges--front").attr('href');
						var split_href = href.split('?');
						$("#view_all_badges--front").attr('href', split_href[0]);

						href = $("#view_all_badges--back").attr('href');
						split_href = href.split('?');
						$("#view_all_badges--back").attr('href', split_href[0]);
					}
				})

				window.returnValidDates = function(data) {
					var output = '';
					for (var x = 0, date; date = data.und[x]; x++) {
						var arr = date.value.split('|');
						if (arr[0] == node.nid) {
							output += Date.parse(arr[1]).toString('ddd, MMM ddS, yyyy');
						}
					}
					return output;
				}
				window.evaluateCheckInOut = function(data) {
					var ts = 0,
						status = 'Never';
					for (var key in data) {
						var datum = data[key];
						if (datum.event_id == node.nid && parseInt(datum.timestamp) > ts) {
							ts = parseInt(datum.timestamp);
							var _ts = new Date(ts * 1000);
							var timestamp = _ts.toString('yyyy-MM-dd h:mm tt');
							status = 'Last Checked ' + datum.type.capitalize() + ' on ' + timestamp;
						}
					}
					return status;
				}
			}
		},
		detach: function() {

		}
	};
})(jQuery);