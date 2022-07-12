/**
 * Provide clientside validation for a specific
 * field collection field, the field_visitor_information field,
 * because Drupal, apparently, cannot handle suchhhh a complex feat.
 */

(function($) {
	Drupal.behaviors.eaton_field_collection_validation = {
	attach: function(context, settings) {
          // override visitor information add another button's click
          //console.log('starting run of eaton\'s field_collection_validation.js');
            var $add_more = $('[name="field_visitor_information_add_more"]');
            if ($add_more.length) {
                var evts = $._data($add_more[0], 'events'), _evts = null;
                if (evts.hasOwnProperty('mousedown')) {
                  _evts = $.extend(true, {}, evts);
                }
                if (_evts != null) {
                	//console.log('our cloned events', _evts);
	                $add_more.unbind().click(function(e) {
	                    e.stopPropagation();
	                    e.preventDefault();
	                   // console.log('clicked add another item');
	                    var errors = [];
	                    var $table = $(this).parents('.field-name-field-visitor-information').find('table.field-multiple-table');
	                    $table.siblings('.messages').remove();
	                    var count = $table.find('tr').length;
	                    var $tr = $table.find('tr').eq(count - 1);
	                    var $visitor_type = $tr.find('.field-name-field-visitor-type select');
	                    var $company_name = $tr.find('.field-name-field-visitor-company-name input');
	                    var $vista_number = $tr.find('.field-name-field-visitor-vista-number input');
	                   // var $vista_unknown = $tr.find('.field-name-field-vista-unknown input');
	                    $visitor_type.siblings('.chosen-container').removeClass('error');
	                    $company_name.removeClass('error');
	                    $vista_number.removeClass('error');
	                   // $vista_unknown.removeClass('error');
	                    if ($visitor_type.val() == '_none') {
	                   		errors.push(['visitor_type','Visitor Type is required.', $visitor_type]);
	                    }
	                    if ($company_name.val().length === 0) {
	                    	errors.push(['company_name', 'Company Name is required.', $company_name]);
	                    }
	                    // if ($vista_number.val().length === 0 && !$vista_unknown.is(':checked')) {
	                    // 	errors.push(['vista_number', 'If the Vista Number is blank, please check the  "VISTA number is unknown." box.', $vista_number]);
	                    // }
	                    if (errors.length) {
	                    	var $messages = $('<div class="messages error" />');
	                    	var $ul = $('<ul class="menu" />');
	                    	for (var i = 0, error; error = errors[i]; i++) {
	                    		//console.log('error on form: ', error);
	                    		var error_class = error[0];
	                    		$ul.append('<li class="' + error_class + '">' + error[1] + '</li>');
	                    		var $el = error[2];
	                    		var $display_el = $el;
	                    		if ($el.is($visitor_type)) {
	                    			$display_el = $visitor_type.siblings('.chosen-container');
	                    		}
	                    		$el.attr('data-classname',error_class);
	                    		$display_el.addClass('error');
	                    		$el.bind('blur change', function(e) {
	                    			var e_class = $(this).attr('data-classname');
	                    			if ($(this).val().length) {
	                    				$('.field-name-field-' + e_class.replace('_','-')).removeClass('error');
	                    				$('.field-name-field-' + e_class.replace('_','-')).children().removeClass('error');
	                    				$('.field-name-field-' + e_class.replace('_','-') + ' .chosen-container').removeClass('error');
	                    				$messages.find('.' + e_class).remove();
	                    				if ($messages.find('ul').children().length === 0) {
	                    					$messages.remove();
	                    				}
	                    			}
	                    		})
	                    	}
	                    	$messages.append($ul);
	                    	$table.before($messages);
	                    	return false;
	                    }
	                    else {	                    
	                      // console.log('can we trigger one of these events we stored?', _evts);
	                     _evts.mousedown[0].handler.call(this);
	                     //Drupal.ajax.eventResponse(e);
	                    }
	                })
            	}
            }
	}
}
})(jQuery);