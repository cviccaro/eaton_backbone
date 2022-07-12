/**
 * This file acts on public registrant form and loads form values based on email
 */
(function($) {
  Drupal.behaviors.email_check = {
    attach: function(context, settings) {
        var $form = $("#eaton-backbone-event-registrant-form");
        var token = Drupal.settings.email_check.security_token;
        if ($form.length) {
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
            // bind to email field
            $('[name="email"]',$form).change(function() {
                if ($(this).val().length) {
                    var val = $.base64.encode($(this).val());
                    $.ajax({
                        url:"/eaton_backbone/rest/attendee_email/" + val + ".json",
                         type:"GET",
                         data:{
                             'tok': token
                         },
                         error:function (jqXHR, textStatus, errorThrown) {
                           console.log('error: ',errorThrown);
                         },
                         success: function (response) {
                            if (response.length && response[0] === false) { 
                                // Remove any previously filled out information if the email is changed and returns no attendee.
                                clearTouched();
                                return; 
                            }
                            var attendee = response;
                            // fill out form
                            for (var key in attendee) {
                               if (key != 'attendee_id' && key != 'created' && key != 'events' && key != 'picture') {
                                    var $field = $('[name="' + key + '"]');
                                    if ($field.length) {
                                        var value = attendee[key];
                                         switch ($field[0].tagName) {
                                             case 'SELECT':
                                                 //console.log('filling out ' + $field[0].tagName + ' field with name ' + key + ' with value ' + value);
                                                 $field.val(value);
                                                 $field.change().trigger('chosen:updated');
                                                 break;
                                             default:
                                                 //console.log('filling out ' + $field[0].tagName + ' field with name ' + key + ' with value ' + value);
                                                 $field.val(value);
                                         }
                                         $field.addClass('touched');
                                    }
                               }                                
                            }
                         } 
                    });
                }
                else {
                    clearTouched();
                }
            });
            function clearTouched() {
                    $form.find('.form-item [name]').each(function() {
                        var key = $(this).attr('name');
                        if (key != 'attendee_id' && key != 'created' && key != 'events' && key != 'picture') {
                            var $field = $('[name="' + key + '"]');
                            if ($field.length && $field.hasClass('touched')) {                                   
                                    $field.val('').removeClass('touched');
                                    if ($field[0].tagName == "SELECT") { $field.change().trigger('chosen:updated'); }
                            }
                        }
                    });
            }
        }
    },
    unattach: function() {
    }
  };
})(jQuery);