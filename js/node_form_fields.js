/**
 * This file handles dynamic form fields on the event request node
 * add/edit page which are outside the capabilites of conditional_field
 */
(function($) {
	Drupal.behaviors.event_node_form_fields = {
		attach: function(context, settings) {
			function militarytoAmPm(time, returnObj) {
				if (typeof(returnObj) == 'undefined') {
					returnObj = false;
				}

				var split_time = time.split(':');
				var hour = parseInt(split_time[0]);
				var min = parseInt(split_time[1]);

				var ampm = 'AM';
				if (hour >= 12) {
					ampm = 'PM';
					if (hour != 12) {
						hour = hour - 12;
					}
				}
				if (hour == 0) {
					hour = 12;
				}
				if (hour.length == 1) {
					hour = '0' + hour;
				}

				if (min.toString().length == 1) {
					min = '0' + min;
				}
				if (returnObj) {
					return {
						'hour': hour,
						'minute': min,
						'ampm': ampm
					};
				} else {
					return hour + ':' + min + ' ' + ampm;
				}
			}
			var timezoneSupport = false,
				timezones = [];
			if (Drupal.settings.hasOwnProperty('eaton_backbone_location_timezones')) {
				timezones = Drupal.settings.eaton_backbone_location_timezones;
				timezoneSupport = true;
			}
			var roomRequirements = {
				pgh: ['datacenter', 'industrial_training_lab', 'power_quality_lab', 'training_room_b', 'residential_area', 'green_technologies', 'cpc_area', 'training_room_d', 'training_room_d_and_e'],
				hou: ['board_room', 'auditorium', 'education_room', 'commercial_training_room', 'industrial_training_room', 'utility_power_distribution_training_room']
			};
			// Use room requirements set from Drupal (new eaton_taxonomies module)
			if (Drupal.settings.hasOwnProperty('eaton_taxonomies_room_requirements')) {
				roomRequirements = Drupal.settings.eaton_taxonomies_room_requirements;
			}
			var purpose_options = {
				pgh: ['training', 'testing', 'facility_tour', 'meeting', 'other'],
				hou: ['training', 'application_overview', 'meeting', 'other']
			};
			// Use visit purposes set from Drupal (new eaton_taxonomies module)
			if (Drupal.settings.hasOwnProperty('eaton_taxonomies_visit_purposes')) {
				purpose_options = Drupal.settings.eaton_taxonomies_visit_purposes;
			}

			// Hide field row if fields inside are hidden
			window.checkFieldRows = function() {
				$('.field-row').each(function() {
					var visible = 0;
					$(this).children().each(function() {
						if ($(this).css('display') !== 'none') {
							visible++;
						}
					});
					if (visible) {
						$(this).show();
					} else {
						$(this).hide();
					}
				});
			}

			$(document).ready(function() {
				// Hide "All Day" options, "Show end date/time" option, and automatically check the "show end/date time box"
				$('.form-item', context).each(function() {
					var className = this.className;
					if (this.className.indexOf('-all-day') != -1) {
						$(this).parent().hide();
					} else if (this.className.indexOf('-show-todate') != -1) {
						$(this).children('input').prop('checked', true).trigger('change');
						$(this).parent().hide();
					}

					$(this).on('change', function() { setTimeout(function() { window.checkFieldRows(); }); });
				})
				// hide js-hide parents
				$('.js-hide', context).each(function() {
					$(this).parent().hide();
				})
				// Hide first row if nothing is visible in it
				var visible_first_row_children = $(".panels-4r-row-first .panels-4r-region-panels-4r-center-inside").children(':visible');
				if (!visible_first_row_children.length) {
					$(".panels-4r-row-first").hide();
				}

				var this_node = Drupal.settings.eaton_node_edit != undefined ? Drupal.settings.eaton_node_edit.node : {};
				// window.this_node = this_node;
				var $locationSelect = $('select[name="field_event_location[und]"]');
				var $roomRequirementContainer = $('#edit-field-room-requirements-und');
				var $purposeOfVisitContainer = $('#edit-field-purpose-of-visit-und');
				var $catering1 = $("#edit-field-psec-catering-und");
				var $catering2 = $("#edit-field-additional-areas-und");
				var $requestStatus = $('[name="field_request_status[und]"]');


				$locationSelect.change(function() {
					var location = $(this).val();
					if (location == 'pgh') {
						$("#edit-field-additional-areas-und").find('input').each(function() {
							$(this).attr('checked', false);
						});
						$("#edit-field-additional-areas-und").change();
					} else if (location == 'hou') {
						$("#edit-field-psec-catering-und").find('input').each(function() {
							$(this).attr('checked', false);
						})
						$("#edit-field-psec-catering-und").change();
					}


					// modify room requirements to location
					$(".form-item", $roomRequirementContainer).each(function() {
						var $input = $(this).find('input');

						if ($.inArray($input.val(), roomRequirements[location]) == -1) {
							//$input.attr('checked',false);
							$(this).hide();
						} else {
							//console.log($input.val());
							$(this).show();
						}
					});
					// modify purpose of visit to location
					$(".form-item", $purposeOfVisitContainer).each(function() {
						var $input = $(this).find('input');
						if ($.inArray($input.val(), purpose_options[location]) == -1) {
							//$input.attr('checked',false);
							$(this).hide();
						} else {
							$(this).show();
						}
					})

					// adjust timezone info
					if (timezoneSupport && timezones.hasOwnProperty(location)) {
						$("#timezone_display", context).html(timezones[location].description);
					}
				});

				// Fill in values..
				if (this_node.hasOwnProperty('field_psec_catering') && this_node.field_psec_catering.hasOwnProperty('und')) {
					var psec_catering = this_node.field_psec_catering.und;
					var catering = [];
					for (var key in psec_catering) {
						catering.push(psec_catering[key].value);
					}
					$('.pane-node-field-psec-catering input[type="checkbox"]').each(function() {
						if ($.inArray($(this).val(), catering) != -1) {
							this.checked = true;
							var that = this;
							setTimeout(function() {
								$(that).attr('checked', true).change();
							}, 1);

							//                      console.log('just checked ', this);
						}
					});
				}
				if (this_node.hasOwnProperty('field_special_request') && this_node.field_special_request.hasOwnProperty('und')) {
					//console.log(this_node.field_special_request);
					setTimeout(function() {
						$(".pane-node-field-special-request").show();
						$(".pane-node-field-special-request input").val(this_node.field_special_request.und[0].value);
					}, 1);
				}
				if (this_node.hasOwnProperty('field_psec_support') && this_node.field_psec_support.hasOwnProperty('und')) {
					if (this_node.field_psec_support.und[0].value != 'no') {
						setTimeout(function() {
							$('.pane-node-field-psec-support-length').show();
						}, 1);
					} else {
						setTimeout(function() {
							$('.pane-node-field-psec-support-length').hide();
						}, 1);
					}
				}
				//console.log(this_node.field_special_request);
				if (this_node.hasOwnProperty('field_special_request') && this_node.field_special_request.hasOwnProperty('und')) {
					$('.pane-node-field-special request input').val(this_node.field_special_request.und[0].value);
				}

				// Conditional fields not handling this properly... do it manually.
				$("#edit-field-psec-catering-und,#edit-field-additional-areas-und").change(function() {
					if ($(this).find('input[value="special"]').length && $(this).find('input[value="special"]').is(':checked')) {
						$("#edit-field-special-request").show();
						$("#edit-field-special-request").parents('.panel-pane').show();
					} else {
						$("#edit-field-special-request").hide();
						$("#edit-field-special-request").parents('.panel-pane').hide();
						//                       $("#edit-field-special-request").find('input').val('');
					}

					window.stylePanels();
				});



				var styleTimer;
				// Hide panel pane when a dependent form field is triggered.
				$(document).bind('state:visible', function(e) {
					if (e.target.className.indexOf('possible-dates') == -1 && e.target.id.indexOf('possible-dates') == -1 &&
						e.target.className.indexOf('suggested-date') == -1 && e.target.id.indexOf('suggested-date') == -1 &&
						e.target.className.indexOf('approved-date') == -1 && e.target.id.indexOf('approved-date') == -1) {
						if ($(e.target).parents('.pane-entity-form-field').length) {
							if (e.value) {
								$(e.target).parents('.pane-entity-form-field').show();
							} else {
								$(e.target).parents('.pane-entity-form-field').hide();
							}
						}
						clearTimeout(styleTimer);
						styleTimer = setTimeout(function() {
							window.stylePanels();
						}, 250);
					}
				})

				window.stylePanels = function stylePanels() {
					$('.panel-pane').removeClass('even odd');
					var i = 0;
					$('.panel-pane:visible').each(function() {
						$(this).addClass((i % 2 == 0) ? 'even' : 'odd');
						i++;
					});
				}

				// Trigger
				$("#edit-field-psec-catering-und:visible,#edit-field-additional-areas-und:visible").change();
				var oldVal = $locationSelect.val() || 'hou';
				if (this_node.nid != undefined) {
					oldVal = this_node.field_event_location.und[0].value;
				}
				$locationSelect.val(-1);
				$locationSelect.change();

				// Trigger state:visible on some dependent elements so the panel-pane (event bound above) will be hidden
				$('#edit-field-psec-support, #edit-field-psec-support-length,#edit-field-additional-areas,#edit-field-psec-support-portion, #edit-field-psec-catering').trigger('state:visible');
				$locationSelect.val(oldVal);
				$locationSelect.change();

				// Attach chosen jquery
				$("#edit-field-request-status-und,#edit-field-event-location-und,#edit-field-event-visibility-und,#edit-field-customer-boolean-und,#edit-field-psec-support-und, #edit-field-psec-support-length-und").chosen({
					disable_search: true
				});
				$(".field-name-field-visitor-type select").chosen({
					disable_search: true
				});

				$('.field-name-field-psec-support select').change(function() {
					if ($(this).val() !== 'yes') {
						$('.field-name-field-psec-support-length select').val('').trigger('state:visible').trigger('chosen:updated');
						$('.field-anme-field-psec-support-portion').val('');
						$('.pane-node-field-psec-support-length, .pane-node-field-psec-support-portion').hide();
						window.checkFieldRows();
					}
				});

				// Show approved date for coordinators
				$("body.coordinator .pane-node-field-approved-date").show();
				//$("body.coordinator .pane-node-field-approved-date .form-type-date-popup:eq(0)").show();
			
				/**
				 * New Node
				 */
				if (!this_node.hasOwnProperty('nid')) {
					// Start Textarea Placeholder
					var placeholders = {
						'field_interests_topics[und][0][value]': 'Describe any interests and topics that will be covered at this event here.',
						'field_sales_opportunities[und][0][value]': 'Describe any sales opportunities related to this event here.',
						'field_room_requirements_extra[und][0][value]': 'Room set-up/configuration, flip charts & markers, slide advancer/pointer, etc.'
					}
					$('textarea').each(function() {
						if ($(this).val() == '') {
							// change value, bind to focus and blur to hide/show value if no text entered
							var key = $(this).attr('name');
							$(this).val(placeholders[key]);
							$(this).focus(function() {
								if ($(this).val() == placeholders[key]) {
									$(this).val('');
								}
							});
							$(this).blur(function() {
								if (!$(this).val().length) {
									$(this).val(placeholders[key]);
								}
							})
						}
					});
					// delete values when form submit is clicked
					$('.form-submit').click(function(e) {
						$('textarea').each(function() {
							var key = $(this).attr('name');
							if ($(this).val() == placeholders[key]) {
								$(this).val('');
							}
						});
						if ($('.pane-node-field-request-status').is(':visible') && $('.pane-node-field-request-status select').val() == 'approved') {
							var passed = true;
							$(".pane-node-field-approved-date input").each(function() {
								if ($(this).attr('type') == 'text' && $(this).is(':visible') && $(this).val() == '') {
									passed = false;
								}
							})
							if (!passed) {
								e.preventDefault();
								e.stopPropagation();
								$('.messages:eq(0) ul').html('<li id="approved_date_error">You must fill out an approved date if creating a new, pre-approved event.</li>');
								$('.messages:eq(0)').show();
								$('.pane-node-field-approved-date .form-wrapper').addClass('error').attr('style', 'background-image:none!important');
								$("html,body").animate({
									scrollTop: 0
								});
								$(".pane-node-field-approved-date input").change(function() {
									if ($(this).val() != '') {
										$("#approved_date_error").remove();
										$('.pane-node-field-approved-date .form-wrapper').removeClass('error');
									}
								})
							}
						}
					});

					// End Textarea Replacements

					/**
					 *  Date Approved -> Possible Date 1
					 *
					 *  Users that can create an event that is already approved have no need for the possible date field.
					 *  This will map whatever a user puts into the Approved Date field right into the Possible Date 1 field.
					 **/
					$("body.coordinator .pane-node-field-approved-date input").once('approvedsupport').each(function() {
						$(this).change(function() {
							var $field = $('[name="' + this.name.replace('field_approved_date', 'field_possible_dates') + '"]');
							if ($field.length) {
								if ($field.attr('type') == 'checkbox') {
									$field.attr('checked', this.checked).change();
								} else if ($field.attr('type') == 'text') {
									$field.val(this.value).change();
								}
								if ($field.hasClass('jp-timepicker-processed')) {
									$field.next().text(militarytoAmPm(this.value));
								}
							}
						});
					});

					window.checkFieldRows();

				}
			}); 
		} // attach
	}
})(jQuery);