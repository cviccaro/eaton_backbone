/* 
 * This file acts on the event request node edit page only
 * Handles deny/approve and preprocessing form fields before allowing page
 * to submit to PHP
 */

(function($) {
  Drupal.behaviors.eaton_node_edit = {
    attach: function() {
        if (Drupal.settings.hasOwnProperty('eaton_node_edit') && Drupal.settings.eaton_node_edit.hasOwnProperty('node')) {
            var $submitButton = $("#event-request-node-form input#edit-submit");
            
            // append "Deny" or "Approve" button next to Request Status
            var $denyButton = $('<button id="deny_event_request">Deny</button>');
            var $approveButton = $('<button id="approve_event_request">Approve</button>');
            
            // hide denied on field
            $(".pane-entity-form-field.pane-node-field-denied-on-date").hide();
            
            var node = Drupal.settings.eaton_node_edit.node;
            var nodeauthor = Drupal.settings.eaton_node_edit.username;
            var original_status = node.field_request_status.und[0].value;
            
            var $requestStatusField = $(".pane-entity-form-field.pane-node-field-request-status .form-item select");
            var $visibilityField = $(".pane-entity-form-field.pane-node-field-event-visibility .form-item select");
            var $deniedOnDateField = $('.pane-entity-form-field.pane-node-field-denied-on-date input');
            var $approvedDateFieldWrapper = $(".pane-entity-form-field.pane-node-field-approved-date").show();
            var $denySuggestedDateWrapper = $(".pane-entity-form-field.pane-node-field-denied-suggested-date");
            var $denyMessageWrapper = $(".pane-entity-form-field.pane-node-field-denied-message");
            var $possibleDatesWrapper = $(".pane-entity-form-field.pane-node-field-possible-dates");
            var defaultButtons = [
                {text: "OK", click: function() { $(this).dialog('close'); } }
            ];
            var defaultDialogOptions = {
                height:"auto",
                width:750,
                modal: true, 
                resizable: false, 
                autoOpen: false, 
                show: {effect: 'blind', duration: 300},
                hide: {effect: 'blind', duration: 300},
                buttons: defaultButtons
            };
            
            var fns = {
              showLoadingIndicator: function() {
                  $("#blinder").show();
              },
              hideLoadingIndicator: function() {
                  $("#blinder").hide();
              }
           };
            
            var dialog;
            if ($("#modal").length) {
                dialog = $("#modal").dialog(defaultDialogOptions);
            }
            else {
                $("body").append('<div id="modal" />');
                dialog = $("#modal").dialog(defaultDialogOptions);
            }
            // add a tag around the request status HTML
            var request_status_html = $(".pane-entity-field.pane-node-field-request-status .pane-content .field").html();
            $(".pane-entity-field.pane-node-field-request-status .pane-content .field").html('<span id="request_status_display">' + request_status_html + '</span>');

            if (original_status != 'denied') {
                $(".pane-entity-form-field.pane-node-field-denied-suggested-date").hide();
                $(".pane-node-field-denied-on-date").hide();
                $(".pane-entity-field.pane-node-field-request-status .pane-content .field").append($denyButton);
                $denyButton.click(function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    // fill in deny value for request status
                    $requestStatusField.val('denied');
                    // click save button for user
                    $submitButton.click();
                })
            }
            if (original_status != 'approved') {
                setTimeout(function() { $approvedDateFieldWrapper.hide(); },0);
                $(".pane-entity-field.pane-node-field-request-status .pane-content .field").append($approveButton);
                $approveButton.click(function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    // fill in deny value for request status
                    $requestStatusField.val('approved');
                    // click save button for user
                    $submitButton.click();
                })                
            }
            if (original_status == 'denied') {
                $('.pane-node-field-denied-message').show();
            }
            else if (original_status == 'approved') {
                if (node.field_denied_suggested_date.hasOwnProperty('und')) {
                    //$('.pane-node-field-denied-suggested-date').show();
                    $(".pane-entity-field.pane-node-field-request-status .pane-content .field #request_status_display").text('Approved (Accepted Suggested Date)');
                }
            }

            // Adjust dates for user TZ
            var dates = node.field_possible_dates;
            var original_dates = $.extend(dates, true);

            var timezone_offset = new Date().getTimezoneOffset() / 60 * -1;
            // if (dates.hasOwnProperty('und')) {
            //   for (var i = 0, d; d = dates.und[i]; i++) {
            //     if (d.timezone_db == 'UTC') {
            //       var start_date = Date.parse(d.value);
            //       d.value = start_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       if (d.hasOwnProperty('value2') && d.value2 != null) {
            //           var end_date = Date.parse(d.value2);
            //           d.value2 = end_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       }
            //       d.timezone_db = Drupal.settings.eaton_backbone.timezone_db;
            //     }
            //   }
            // }

            var denied_date = node.field_denied_suggested_date;
            // if (denied_date.hasOwnProperty('und')) {
            //   if (denied_date.und[0].timezone_db == 'UTC') {
            //     var start_date = Date.parse(denied_date.und[0].value);
            //       denied_date.und[0].value = start_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       if (denied_date.und[0].hasOwnProperty('value2') && denied_date.und[0].value2 != null) {
            //         var end_date = Date.parse(denied_date.und[0].value2);
            //         denied_date.und[0].value2 = end_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       }
            //       denied_date.und[0].timezone_db = Drupal.settings.eaton_backbone.timezone_db;
            //       //node.set('field_denied_suggested_date', denied_date);
            //    }
            // }    
          var approved_date = node.field_approved_date;
            // if (approved_date.hasOwnProperty('und')) {

            //   if (approved_date.und[0].timezone_db == 'UTC') {
            //     var start_date = Date.parse(approved_date.und[0].value);
            //       approved_date.und[0].value = start_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       if (approved_date.und[0].hasOwnProperty('value2') && approved_date.und[0].value2 != null) {
            //         var end_date = Date.parse(approved_date.und[0].value2);
            //         approved_date.und[0].value2 = end_date.addHours(timezone_offset).toString('yyyy-MM-dd HH:mm:s');
            //       }
            //       approved_date.und[0].timezone_db = Drupal.settings.eaton_backbone.timezone_db;
            //       //node.set('field_denied_suggested_date', denied_date);
            //    }
            // }                


            $("#event-request-node-form input#edit-submit").click(function(e) {
                var new_status = $requestStatusField.val();
                // get data from button.  if this isnt set, the form wont save.. ever.
                var validate_ok = $(this).data('validate_ok') != undefined ? $(this).data('validate_ok') : false;
                //console.log('validate OK? ', validate_ok);
                if (original_status != 'denied' && new_status == 'denied' && !validate_ok) {
                    e.preventDefault();
                    e.stopPropagation();
                    // prompt about suggesting a date.
                    var dates = node.field_possible_dates, olddates = '<ul>';
                    for (var x = 0, date; date = dates.und[x]; x++) {
                        olddates += '<li>' + dateRange(date) + '</li>';
                    }
                    olddates += '</ul>';
                    // get form field already existant, detach it.
                    $denySuggestedDateWrapper = $denySuggestedDateWrapper.show().detach();
                    $denyMessageWrapper = $denyMessageWrapper.show().detach();
                    $denyMessageWrapper.find('textarea').val('');
                    var $denyMarkup = $('<div />');
                    $denyMarkup.append('<p>To deny an event request, you are confirming the following dates are not available for the initiator</p>' + olddates + '<p>Choose a new possible date for the event initiator, and click "Deny".</p>');
                    dialog.html('').append($denyMarkup);

                    var $denyForm = $('<form id="deny-event-request-form" />');//<label for="new-date">New Date</label><input type="text" name="new-date" /></form>
                    $denyMarkup.append($denyForm);
                    $denyForm.append($denySuggestedDateWrapper);
                    $denyForm.append($denyMessageWrapper);
                    
                    var $denyValidationWrapper = $('<div id="dialog-errors" class="dialog-errors" style="display:none;" />');
                    $denyForm.prepend($denyValidationWrapper);
                    
                    dialog.dialog('option',{
                        title: 'Event Denial',
                        buttons: [
                            {
                                text: "Deny", 
                                click: function() { 
                                    var errors = [];
                                    $denyValidationWrapper.empty();
                                    var suggestion = {
                                        all_day: $(this).find('[name="field_denied_suggested_date[und][0][all_day]"]')[0].checked,
                                        start: {
                                            date: $(this).find('[name="field_denied_suggested_date[und][0][value][date]"]').val()
                                        }
                                    }
                                    if (!suggestion.all_day) {
                                        suggestion.start.time = $(this).find('[name="field_denied_suggested_date[und][0][value][time]"]').val();
                                        if (!suggestion.start.time.match(/^\d{2,}:\d{2}$/)) {
                                            errors.push('Invalid time entered for start time.');
                                        }                                        
                                    }
                                    if (suggestion.start.date == '') {
                                        errors.push("The start date cannot be empty.");
                                    }
                                    else {
                                        var has_end = $(this).find('[name="field_denied_suggested_date[und][0][show_todate]"]')[0].checked;
                                        if (has_end) {
                                            suggestion.end = {
                                                date: $(this).find('[name="field_denied_suggested_date[und][0][value2][date]"]').val()
                                            }
                                            if (!suggestion.all_day) {
                                                suggestion.end.time = $(this).find('[name="field_denied_suggested_date[und][0][value2][time]"]').val();
                                                if (!suggestion.end.time.match(/^\d{2,}:\d{2}$/)) {
                                                    errors.push('Invalid time entered for end time.');
                                                }
                                            }
                                        }
                                        if (has_end && suggestion.end.date == '') {
                                            errors.push("The end date cannot be empty.");
                                        }
                                        else {
                                            if (suggestion.all_day) {
                                                // no times
                                                suggestion.start.obj = Date.parse(suggestion.start.date);
                                                if (has_end) {
                                                    suggestion.end.obj = Date.parse(suggestion.end.date);
                                                }
                                            }
                                            else {
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
                                                if (suggestion.end.obj.isBefore(suggestion.start.obj)) {
                                                    errors.push("The end date cannot be before the start date.");
                                                }
                                            }                                            
                                        }
                                    }
                                   
                                      
                                    if (errors.length) {
                                        var errorList = arrayToUL(errors);
                                        $denyValidationWrapper.append(errorList);
                                        $denyValidationWrapper.show("blind",300);
                                    }
                                    else {
                                        // no errors
                                        $denyValidationWrapper.hide("blind",300);
                                        // restore field to form
                                        $('.panels-4r-region.administrative-items').append($denySuggestedDateWrapper.hide());
                                        $('.panels-4r-region.administrative-items').append($denyMessageWrapper.hide());
                                        // set denied on date
                                        $deniedOnDateField.val(Date.today().toString('yyyy-MM-dd'));
                                        // clear approved date
                                        $approvedDateFieldWrapper.find('input').each(function() {
                                           var $approvedDateData = $approvedDateFieldWrapper.data('original_values');
                                           if ($approvedDateData == undefined) {
                                            $approvedDateData = {};
                                           }
                                           var type = $(this).attr('type');
                                           if (type == 'checkbox') {
                                                $approvedDateData[this.name] = this.checked;
                                               $(this).attr('checked',false);
                                           }
                                           else if (type == 'text') {
                                                $approvedDateData[this.name] = this.value;
                                               $(this).val('');
                                           }
                                           $approvedDateFieldWrapper.data('original_values',$approvedDateData);
                                        });
                                        // append data to button, click again
                                        $submitButton.data('validate_ok',true);
                                        $(this).dialog('close');
                                        window.loadingIndicator.show();
                                        $submitButton.click();
                                    }
                                    //console.log('suggested date is ', suggestion);
                                }
                            },  
                            {
                                text: "Cancel", 
                                    click: function() { 
                                        $(this).dialog('close'); 
                                        // restore request status value
                                        //console.log('original values',$approvedDateFieldWrapper.data('original_values'));
                                        $requestStatusField.val(original_status);
                                        // restore approved date 
                                        if ($approvedDateFieldWrapper.data('original_values') != undefined) {
                                            var original_values = $approvedDateFieldWrapper.data('original_values');
                                            for (var name in original_values) {
                                                var original_value = original_values[name];
                                                var $element = $approvedDateFieldWrapper.find('[name="' + name + '"]');
                                                if ($element[0].tagName == 'INPUT' && $element[0].type == 'checkbox') {
                                                    if (original_value) {
                                                        $element.attr('checked',true);
                                                    }
                                                    else {
                                                        $element.attr('checked', null);
                                                    }
                                                }
                                                else if ($element[0].tagName == 'INPUT' && $element[0].type == 'text') {
                                                    $element.val(original_value);
                                                }
                                            }
                                        }
                                        // restore field to form
                                        $('.panels-4r-region.administrative-items').append($denySuggestedDateWrapper.hide());
                                        $('.panels-4r-region.administrative-items').append($denyMessageWrapper.hide());
                                    } 
                            }
                        ], 
                        open: function() {
                            //$("#deny-event-request-form input").datepicker(); 
                            // var timeSettings = {
                            //     fromTo: false,
                            //     show24Hours: true,
                            //     showSeconds: false,
                            //     spinnerImage: "",
                            //     timeSteps: [1,15,0]
                            // };
                            // $('[name="field_denied_suggested_date[und][0][value][time]"]').timeEntry(timeSettings);
                            // $('[name="field_denied_suggested_date[und][0][value2][time]"]').timeEntry(timeSettings);
                            dialog.parent().addClass('deny-dialog');
                            dialog.parent().find('.timepicker-input-display').each(function() {
                                var $display = $(this);
                                var $input = $(this).prev();
                                $display.outerWidth($input.outerWidth() - 2);
                                $display.outerHeight($input.outerHeight() - 2);
                                $display.position({my: 'center center', at: 'center center', of: $input});
                            })
                            // dialog.parent().find('[name="field_denied_suggested_date[und][0][show_todate]"]').change(function() {
                            //     //console.log('hit show todate in deny dialog');
                            //     var $input = dialog.parent().find('[name="field_denied_suggested_date[und][0][value2][time]"]');
                            //     var $display = $input.next();
                            //     $display.outerWidth($input.outerWidth() - 2);
                            //     $display.outerHeight($input.outerHeight() - 2);
                            //     $display.position({my: 'center center', at: 'center center', of: $input});
                            // })    
                            // dialog.parent().find('[name="field_denied_suggested_date[und][0][all_day]"]').change(function() {
                            //     //console.log('hit all day in deny dialog');
                            //     var $input = dialog.parent().find('[name="field_denied_suggested_date[und][0][value][time]"]');
                            //     var $display = $input.next();
                            //     $display.outerWidth($input.outerWidth() - 2);
                            //     $display.outerHeight($input.outerHeight() - 2);
                            //     $display.position({my: 'center center', at: 'center center', of: $input});                                
                            //     var $input = dialog.parent().find('[name="field_denied_suggested_date[und][0][value2][time]"]');
                            //     var $display = $input.next();
                            //     $display.outerWidth($input.outerWidth() - 2);
                            //     $display.outerHeight($input.outerHeight() - 2);
                            //     $display.position({my: 'center center', at: 'center center', of: $input});
                            // });
                            
                        },
                        close: function() {
                            dialog.parent().removeClass('deny-dialog');
                        }
                    });
                    dialog.dialog("open");
                }
                else if (new_status == 'approved' && original_status != 'approved' && !validate_ok) {
                    e.preventDefault();
                    e.stopPropagation();
                    // prompt user to select which of the 3 dates they are approving (or suggested date)
                    
                    var dates = $.extend(true,{},node.field_possible_dates);
                   
                    //console.log(dates);

                    var accepted = node.hasOwnProperty('suggested_date_accepted') ? node.suggested_date_accepted : null;

                    var options = '';
                    for (var x = 0, date; date = dates.und[x]; x++) {
                        options += '<option value="' + x + '">' + dateRange(date) + '</option>';
                    }
                    if (typeof(node.field_denied_suggested_date.und) != "undefined") {
                        // add denied sugggeste date to options
                         if (accepted != null) {
                            options += '<option value="suggested" selected>Suggested Date: ' + dateRange(node.field_denied_suggested_date.und[0]) + '</option>';
                        }
                        else {
                            options += '<option value="suggested">Suggested Date: ' + dateRange(node.field_denied_suggested_date.und[0]) + '</option>';    
                        }
                        
                    }
                    // configure dialog
                    dialog.dialog('option',{
                        title: 'Choose a date to approve',
                        buttons:[
                            {text: "Approve", click: function() { 
                                    var date_delta = $(this).find('#choose-date').val();
                                    var $date_wrapper = '';
                                    var approved_date = '';
                                    var selector = '';
                                    if (date_delta == 'suggested') {
                                        approved_date = node.field_denied_suggested_date.und[0];
                                        $date_wrapper = $denySuggestedDateWrapper.find('.fieldset-wrapper');
                                        selector = 'field_denied_suggested_date[und][0]';
                                    }
                                    else {
                                        date_delta = parseInt(date_delta);
                                        approved_date = original_dates.und[parseInt(date_delta)];
                                        $date_wrapper = $possibleDatesWrapper.find('.field-multiple-table tbody').find('tr').eq(date_delta).find('td').eq(1);
                                        selector = 'field_possible_dates[und][' + date_delta + ']';
                                    }
                                    
                                    var all_day = false,
                                        has_end = false;
                                       //  console.log({
                                       //      "date_delta": date_delta,
                                       //      wrapper: $date_wrapper,
                                       //      inputs: $date_wrapper.find('input'),
                                       //      selector: selector,
                                       //      parent: $date_wrapper.find('[name="' + selector + '"]'),
                                       //      show_todate: $date_wrapper.find('[name="' + selector + '[show_todate]"]'),
                                       //      all_day:$date_wrapper.find('[name="' + selector + '[all_day]"]')
                                       // });
                                    if ($date_wrapper.find('[name="' + selector + '[show_todate]"]').is(':checked')) {
                                        has_end = true;
                                    }
                                   if ($date_wrapper.find('[name="' + selector + '[all_day]"]').is(':checked')) {
                                        all_day = true;
                                    }                                    
                                    //console.log('chose date delta ' + date_delta + '.  Equals ', approved_date,'.  Has end? ' , has_end, ' All Day? ',all_day);
                                    // var approved_date = $(this).find('#choose-date').val();
                                    // var has_end = approved_date.indexOf("(No End)") == -1 ? true : false;
                                    // var all_day = approved_date.indexOf('(All Day)') == -1 ? false : true;
                                    if (has_end) {
                                        $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][show_todate]"]').attr('checked',true).change();
                                        var dateArr = approved_date.value.split(' ');
                                        var startDate = dateArr[0], startTime = dateArr[1],
                                            endDate = '', endTime = '';
                                        $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value][date]"]').val(startDate);
                                        if (approved_date.hasOwnProperty('value2') && approved_date.value2 != null) {
                                            var dateArr = approved_date.value2.split(' ');
                                            endDate = dateArr[0];
                                            endTime = dateArr[1];
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value2][date]"]').val(endDate);
                                        }
                                        
                                        if (all_day) {
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][all_day]"]').attr('checked',true).change();
                                        }
                                        else {
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][all_day]"]').attr('checked',null).change();
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value][time]"]').val(startTime.substr(0,5));
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value2][time]"]').val(endTime.substr(0,5));
                                        }
                                        
                                    }
                                    else {
                                        $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][show_todate]"]').attr('checked',false).change();
                                        var dateArr = approved_date.value.split(' ');
                                        var startDate = dateArr[0], startTime = dateArr[1];
                                        $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value][date]"]').val(startDate); 
                                        if (all_day) {
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][all_day]"]').attr('checked',true).change();
                                        }
                                        else {
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][all_day]"]').attr('checked',null).change();
                                            $approvedDateFieldWrapper.find('[name="field_approved_date[und][0][value][time]"]').val(startTime.substr(0,5));
                                        }
                                    }
                                    
                                    // clear deny times/dates
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][value][date]"]').val('');
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][value][time]"]').val('');
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][value2][date]"]').val('');
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][value2][time]"]').val('');
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][show_todate]"]').attr('checked',false);
//                                    $denySuggestedDateWrapper.find('[name="field_denied_suggested_date[und][0][all_day]"]').attr('checked',false);
//                                    $deniedOnDateField.val('');
//                                    $denyMessageWrapper.find('textarea').val('');
//                                    
                                  var $approve_private = dialog.parent().find('[name="approve_private_request"]');
                                    if ($approve_private.length) {
                                        if ($approve_private.is(':checked')) {
                                            $visibilityField.val('private').change().trigger('chosen:updated');
                                        }
                                        else {
                                            $visibilityField.val('public').change().trigger('chosen:updated');   
                                        }
                                    }
                                    $submitButton.data('validate_ok',true);
                                    $submitButton.click();
                                }
                            },
                            {text: "Cancel", click: function() { 
                                    // restore request status value
                                    $requestStatusField.val(original_status);
                                    $(this).dialog('close'); 
                                }
                            }
                        ],
                        open: function() {
                            dialog.parent().addClass('approve-dialog');
                        },
                        close: function() {
                            dialog.parent().removeClass('approve-dialog');
                        }                                
                    });
                    // build dialog content
                    var html = '';
                   // html += '<h3>Editing Event Request <em>"' + node.title + '"</em> created by <em> ' + nodeauthor + '</em></h3>';
                    // notify coordinator if request was previously denied.
                    if (original_status == 'denied') {
                        html += '<p class="approve-note">Note: This request was previously denied.  Approving this request will reset the denial.</p>';
                        if (accepted != null) {
                              var accepted_ts = datestampToDate(accepted.timestamp);
                              html += '<p class="approve-note">Note: The initiator has accepted the suggested date on ' + accepted_ts + '.  It has been selected as the default choice for approval below.</p>';
                        }
                    }
                    // build form
                    html += '<p>Select one of the three possible dates chosen by <em>' + nodeauthor + '</em>, or the suggested date when denied (if available), then click Approve.</p><form id="choose-date-form"><label for="choose-date">Possible Dates</label><select id="choose-date" name="choose-date">' + options + '</select></form>';
                    var requested_private = node.field_request_private_event;
                    requested_private = requested_private.hasOwnProperty('und') && requested_private.und[0].value == '1';
                    if (requested_private) {
                        html += '<div class="field field-field-request-private-event"><input type="checkbox" name="approve_private_request" id="approve-private-request" /><label for="approve_private_request">This event was requested to be private.  Check here to approve this event as private.</label></div>';
                    }
                    dialog.html(html);
                    dialog.dialog('open');
                }
                else {
                    // OK, show loading dialog
                    window.loadingIndicator.show();
                }
            })
            var arrayToUL = function arrayToUL(arr) {
                var ul = '<ul>';
                for (var i = 0, x; x = arr[i]; i++) {
                    ul += '<li>' + x + '</li>';
                }
                ul += '</ul>';
                return ul;
            }

            
        }
    },
    unattach: function() {

    }
  }
})(jQuery);
