(function($) {
	Drupal.behaviors.eaton_today_page = {
		attach: function(context, settings) {
			// Configuration and functions
			var config = {
				restEndpoint: '/eaton_backbone/rest',
				defaultDialogOptions: {
					height: "auto",
					width: "auto",
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
					buttons: [{
						text: "OK",
						click: function() {
							$(this).dialog('close');
						}
					}],
					close: function() {},
					open: function() {}
				},
				fns: {
					launchDialog: function(options, html) {
						var dialog = appState.dialog;
						if (dialog.dialog('isOpen')) {
							dialog.dialog('close');
						}
						// reset default options
						dialog.dialog('option', config.defaultDialogOptions);
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
						config.fns.launchDialog({
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
						}, markup);
					},
					launchErrorDialog: function() {
						config.fns.launchMessageDialog('An error occured.  Please try again later or contact support.', 'alert')
					},
					closeDialog: function() {
						appState.dialog.dialog('close');
					}
				}
			};

			// Create modal if it doesn't exist
			if (!$("#modal").length) {
				$("body").append('<div id="modal" />');
			}

			// Create appState
			var appState = {
				dialog: $("#modal").dialog(config.defaultDialogOptions)
			};
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

			// Function to evaulate check_status data property (set during attendeeresource_retrieve in attendeeresource.module)
			window.evaluateCheckInOut = function(data, nid) {
				var ts = 0,
					status = 'Never';
				for (var key in data) {
					var datum = data[key];
					if (datum.event_id == nid && parseInt(datum.timestamp) > ts) {
						ts = parseInt(datum.timestamp);
						var _ts = new Date(ts * 1000);
						var timestamp = _ts.toString('ddd, yyyy-MM-dd h:mm tt');
						status = 'Last Checked ' + datum.type.capitalize() + ' on ' + timestamp;
					}
				}
				return status;
			}

			// Iterate through event attendee divs and attach to them
			$(".eaton-backbone-event-attendees").once('attendees').each(function() {
				var $op_td = $(this).find('.field-operations');
				if ($op_td.length && $op_td.children().length) {
					// Click handler for any link in operations column
					$op_td.children().once('attendee-operations').each(function() {
						$(this).click(function(e) {
							e.preventDefault();
							e.stopPropagation();
							var that = this,
								attendee_id = $(this).parents('tr').attr('data-id'),
								nid = $(this).parents('table').attr('data-id'),
								eventTitle = $(this).parents('.eaton-backbone-event-attendees').find('h2').text(),
								AttendeeModel = Drupal.Backbone.Models.Attendee, // from attendeeresource-services.js
								attendee = new AttendeeModel({
									attendee_id: attendee_id
								}),
								attendeeName = $(this).parents('tr').find('td.field-name').text(),
								attendeeEmail = $(this).parents('tr').find('td.field-email').text(),
								$checkStatus = $(this).parents('tr').find('.field-checked-in'),
								method = '',
								dialogTitle = 'Are you sure?',
								dialogHtml = '',
								OKButtonText = 'OK',
								OKButtonClass = '';

							if ($(this).hasClass('email-button')) {
								method = 'email';
								dialogHtml = 'Are you sure you want to send out a registration email to <em>' + attendeeName + '(' + attendeeEmail + ')' + '</em> for event <em>' + eventTitle + '</em>?';
							} else if ($(this).hasClass('check-inout-button')) {
								if ($(this).hasClass('check-in')) {
									method = 'check in';
									OKButtonText = 'Check In';
									OKButtonClass = 'check-in';
								} else {
									method = 'check out';
									OKButtonText = 'Check Out';
									OKButtonClass = 'check-out';
								}
								dialogHtml = 'Are you sure you want to ' + method + ' <em>' + attendeeName + '(' + attendeeEmail + ')' + '</em>' + ' to event <em>' + eventTitle + '</em>?';
							}
							config.fns.launchDialog({
								title: dialogTitle,
								buttons: [{
									text: OKButtonText,
									'class': OKButtonClass,
									click: function() {
										$(this).dialog('close');
										attendee.fetch({
											success: function(model, response, options) {
												// Call dispatchEmail method via backbone
												if (method == 'email') {
													model.dispatchEmail(config.restEndpoint, nid, function() {
														var html = 'A preregistration email has been successfully sent to attendee <em>' + model.getLabel() + '</em> for event <em>' + eventTitle + '</em>';
														config.fns.launchMessageDialog(html, 'success');

													}, function() {
														console.log('error dispatching email', arguments);
														config.fns.launchErrorDialog();
													})
												} else if (method == 'check in') {
													// Call check in method via backbone
													model.checkIn(config.restEndpoint, nid, function() {
														// Fetch attendee again to get updated check_status
														model.fetch({
															success: function() {
																var html = 'Attendee <em>' + model.getLabel() + '</em> has been successfully checked into event <em>' + eventTitle + '</em>.';
																$checkStatus.html(evaluateCheckInOut(model.get('check_status'), nid));
																config.fns.launchMessageDialog(html, 'check-in');
															}
														})

													}, function() {
														console.log('error checking attendee in', arguments);
														config.fns.launchErrorDialog();
													})
												} else if (method == 'check out') {
													// Call check out method via backbone
													model.checkOut(config.restEndpoint, nid, function() {
														// Fetch attendee again to get updated check_status
														model.fetch({
															success: function() {
																var html = 'Attendee <em>' + model.getLabel() + '</em> has been successfully checked out of event <em>' + eventTitle + '</em>.';
																$checkStatus.html(evaluateCheckInOut(model.get('check_status'), nid));
																config.fns.launchMessageDialog(html, 'check-out');
															}
														})

													}, function() {
														console.log('error checking attendee out', arguments);
														config.fns.launchErrorDialog();
													})
												}
											}
										})
									}
								}, {
									text: 'Cancel',
									click: function() {
										config.fns.closeDialog();
									}
								}],
							}, dialogHtml);
						})
					})
				}
			});
		}
	}
})(jQuery);