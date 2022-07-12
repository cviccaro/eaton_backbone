<?php

function eaton_backbone_admin_eaton_locations() {
	$form['messages'] = array('#markup' => theme_status_messages(array('display' => null))); // Display error messages

	$destination = drupal_get_destination();
	$link_options = array('query' => array('destination' => $destination['destination']));
	$form['add_link'] = array('#markup' => '<div style="margin:0 0 20px 0">' . l('Add New Location', 'admin/config/eaton/locations/add', $link_options) . '</div>');

	$header = array(
		'name' => array('data' => 'Name', 'field' => 'l.name'),
		'code' => array('data' => 'Code', 'field' => 'l.code'),
		'contact' => array('data' => 'Contact', 'field' => 'l.contact'),
		'contact_title' => array('data' => 'Contact Title', 'field' => 'l.contact_title'),
		'email' => array('data' => 'Email', 'field' => 'l.email'),
		'phone' => array('data' => 'Phone', 'field' => 'l.phone'),
		'events_enabled' => array('data' => 'Events Enabled', 'field' => 'l.events_enabled'),
		'created' => array('data' => 'Created', 'field' => 'l.created'),
		'changed' => array('data' => 'Last Updated', 'field' => 'l.changed', 'sort' => 'desc'),
		'operations' => array('data' => 'Operations'),
	);
	$options = array();
	$locations = eaton_backbone_eaton_locations();
	if (!empty($locations)) {
		foreach ($locations as $location) {
			$options[$location->location_id] = array(
				'name' => $location->location_name,
				'code' => $location->location_code,
				'contact' => $location->location_contact,
				'contact_title' => $location->location_contact_title,
				'email' => $location->location_email,
				'phone' => $location->location_phone,
				'events_enabled' => $location->location_available_for_events ? 'Yes' : 'No',
				'created' => '<time class="timeago" datetime="' . format_date($location->created, 'custom', DATE_ISO8601) . '">' . ($location->created) . '</time>',
				'changed' => '<time class="timeago" datetime="' . format_date($location->changed, 'custom', DATE_ISO8601) . '">' . ($location->changed) . '</time>',
				'operations' => l('Edit', 'admin/config/eaton/locations/edit/' . $location->location_id, $link_options) . '<br />' . l('Delete', 'admin/config/eaton/locations/delete/' . $location->location_id, $link_options),
			);
		}
	}
	$sort = tablesort_get_sort($header);
	$order = tablesort_get_order($header);
	$order_sql = str_replace('l.', '', $order['sql']);
	switch ($sort) {
	case 'desc':
		uasort($options, function ($a, $b) use ($order_sql) {
			switch ($order_sql) {
			case 'name':
				return -1 * strcmp(strip_tags($a[$order_sql]), strip_tags($b[$order_sql]));
				break;
			case 'created':
			case 'changed':
				return strip_tags($a[$order_sql]) > strip_tags($b[$order_sql]) ? -1 : 1;
				break;
			default:
				return $a[$order_sql] > $b[$order_sql] ? -1 : 1;
			}

		});
		break;
	case 'asc':
		uasort($options, function ($a, $b) use ($order_sql) {
			switch ($order_sql) {
			case 'name':
				return strcmp(strip_tags($a[$order_sql]), strip_tags($b[$order_sql]));
				break;
			case 'created':
			case 'changed':
				return strip_tags($a[$order_sql]) > strip_tags($b[$order_sql]) ? 1 : -1;
				break;
			default:
				return $a[$order_sql] > $b[$order_sql] ? 1 : -1;
			}
		});
		break;
	}

	$form['locations'] = array(
		'#type' => 'tableselect',
		'#header' => $header,
		'#options' => $options,
		'#empty' => t('No Eaton locations exist yet.'),
		'#js_select' => true,
		'#attributes' => array(
			'class' => array('admin-table'),
		),
	);

	$timeago_path = drupal_get_path('module', 'eaton_backbone') . '/js/jquery.timeago.js';
	if (file_exists($timeago_path)) {
		drupal_add_js($timeago_path);
		drupal_add_js('jQuery(document).ready(function() {  jQuery(".timeago").timeago(); });', 'inline');
	}
	$form['pager'] = array('#markup' => theme('pager'));
	return $form;
}

function eaton_backbone_admin_eaton_locations_form($form, $form_state, $location_id = null) {
	$location = false;
	$submit_text = 'Add Location';
	if (!is_null($location_id)) {
		$location = db_select('eaton_backbone_eaton_locations', 'l')->fields('l')->condition('location_id', $location_id)->execute()->fetchObject();
		$submit_text = 'Save Changes';
	} else {
		$form['#validate'] = array('eaton_backbone_admin_eaton_locations_form_add_validate');
	}
	module_load_include('install', 'eaton_backbone', 'eaton_backbone');
	$schema = eaton_backbone_schema();
	$config = $schema['eaton_backbone_eaton_locations'];
	$weight = -20;
	foreach ($config['fields'] as $field_name => $field) {
		$field_label = ucwords(implode(' ', explode('_', $field_name)));
		switch ($field_name) {
		case 'created':
		case 'changed':
			// ignore these columns
			break;
		case 'location_id':
			if ($location) {
				$form[$field_name] = array('#type' => 'value', '#value' => $location_id);
			}
			break;
		case 'location_available_for_events':
			$form[$field_name] = array(
				'#type' => 'checkbox',
				'#title' => $field_label,
				'#description' => t('Checking this box enables the location to be used for event requests.  Otherwise, it will only be used in the contact form.'),
				'#weight' => 20,
			);
			break;
		case 'badge_front':
			$form[$field_name] = array(
				'#type' => 'managed_file',
				'#title' => t('Front of Badge'),
				'#upload_location' => 'public://eaton_backbone/badging/front',
				'#upload_validators' => array(
					'file_validate_extensions' => array('png jpg jpeg'),
				),
				'#weight' => 30,
			);
			break;
		case 'badge_back':
			$form[$field_name] = array(
				'#type' => 'managed_file',
				'#title' => t('Back of Badge'),
				'#upload_location' => 'public://eaton_backbone/badging/back',
				'#upload_validators' => array(
					'file_validate_extensions' => array('png jpg jpeg'),
				),
				'#weight' => 40,
			);
			break;
		case 'qr_size':
			// add qr size option
			$form[$field_name] = array(
				'#type' => 'select',
				'#options' => drupal_map_assoc(array('normal', 'large')),
				'#title' => 'QR Code Size',
				'#weight' => 49,
			);
			break;
		case 'timezone':
			// add timezone dropdown
			$form[$field_name] = array(
				'#title' => t('Location Timezone'),
				'#type' => 'select',
				'#options' => eaton_backbone_timezone_enabled_identifiers_list(),
				'#required' => true,
				'#description' => t('Select the timezone this location is in.'),
			);
			break;
		case 'address':
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => 'Location ' . $field_label,
				'#required' => false,
				'#weight' => 6,
			);
			break;
		case 'city':
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => 'Location ' . $field_label,
				'#required' => false,
				'#weight' => 7,
			);
			break;
		case 'state':
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => 'Location ' . $field_label,
				'#required' => false,
				'#weight' => 8,
			);
			break;
		case 'postal':
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => 'Location ' . $field_label,
				'#required' => false,
				'#weight' => 9,
			);
			break;
		case 'country':
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => 'Location ' . $field_label,
				'#required' => false,
				'#weight' => 10,
			);
			break;
		default:
			// dpm($field_name);
			$weight = $weight + 5;
			$form[$field_name] = array(
				'#type' => 'textfield',
				'#title' => $field_label,
				'#required' => true,
				'#weight' => $weight,
			);
			if ($field_name == 'location_code') {
				$form[$field_name]['#maxlength'] = 3;
				$form[$field_name]['#description'] = t('3 characters maximum.');
			}
		}
		if ($location) {
			$form[$field_name]['#default_value'] = $location->{$field_name};
			$file_fields = array('badge_front', 'badge_back');
			foreach ($file_fields as $f_name) {
				if (!empty($location->{$f_name}) && $f_name == $field_name) {
					// Show preview
					$human_name = ucwords(str_replace('_', ' ', $f_name));
					$file = file_load($location->{$f_name});
					$form[$f_name . '_preview'] = array(
						'#prefix' => '<h2>Preview ' . $human_name . ' File</h2>',
						'#markup' => theme_image(array('path' => file_create_url($file->uri), 'width' => '', 'height' => '', 'alt' => 'Preview', 'attributes' => array())),
						'#weight' => $form[$f_name]['#weight'] + 5,
					);
				}
			}
		}
	}
	$form['submit'] = array('#type' => 'submit', '#value' => $submit_text, '#weight' => 50, '#prefix' => '<div>');
	$form['cancel'] = array('#type' => 'submit', '#value' => t('Cancel'), '#limit_validation_errors' => array(), '#weight' => 60, '#suffix' => '</div>');
	return $form;
}

function eaton_backbone_admin_eaton_locations_form_add_validate($form, &$form_state) {
	$location_code = $form_state['values']['location_code'];
	$location = eaton_backbone_eaton_location_by_code($location_code);
	if ($location) {
		form_set_error('location_code', t('Location Code @code already exists.', array('@code' => $location_code)));
	}
}

function eaton_backbone_admin_eaton_locations_form_submit($form, &$form_state) {
	switch ($form_state['triggering_element']['#value']) {
	case 'Save Changes':
		$form_state['redirect'] = 'admin/config/eaton/locations/locations';
		$values = $form_state['values'];
		foreach ($values as $key => $val) {
			if (is_array($val)) {
				$values[$key] = serialize($val);
			}
		}
		$record = $values;
		// add timestamp
		$record['changed'] = time();
		drupal_write_record('eaton_backbone_eaton_locations', $record, array('location_id'));
		$msg = t('Eaton Location <em>@name (@code)</em> has been successfully updated.', array('@name' => $values['location_name'], '@code' => $values['location_code']));
		drupal_set_message($msg);
		$file_fields = array('badge_front', 'badge_back');
		foreach ($file_fields as $field_name) {
			if (isset($values[$field_name]) && !empty($values[$field_name])) {
				$file = file_load($values[$field_name]);
				$file->status = FILE_STATUS_PERMANENT;
				file_save($file);
				file_usage_add($file, 'eaton_backbone', 'location', $values['location_id']);
			}
		}
		break;
	case 'Add Location':
		$form_state['redirect'] = 'admin/config/eaton/locations/locations';
		$values = $form_state['values'];
		foreach ($values as $key => $val) {
			if (is_array($val)) {
				$values[$key] = serialize($val);
			}
		}
		$record = $values;
		// add created timestamp
		$record['created'] = $record['changed'] = time();
		drupal_write_record('eaton_backbone_eaton_locations', $record);
		$msg = t('Eaton Location <em>@name (@code)</em> has been successfully added.', array('@name' => $values['location_name'], '@code' => $values['location_code']));
		drupal_set_message($msg);
		$file_fields = array('badge_front', 'badge_back');
		foreach ($file_fields as $field_name) {
			if (isset($values[$field_name]) && !empty($values[$field_name])) {
				$file = file_load($values[$field_name]);
				$file->status = FILE_STATUS_PERMANENT;
				file_save($file);
				file_usage_add($file, 'eaton_backbone', 'location', $values['location_id']);
			}
		}
		break;
	case 'Cancel':
		$form_state['redirect'] = 'admin/config/eaton/locations/locations';
		break;
	}
}

function eaton_backbone_admin_eaton_locations_form_delete($form, &$form_state, $location_id) {
	$location = eaton_backbone_eaton_location_by_id($location_id);
	$info_text = t('<h2>Are you sure you want to remove the Eaton Location <em>@name (@code)</em> from the system?  <strong>This cannot be undone.</strong></h2>', array('@name' => $location->location_name, '@code' => $location->location_code));
	$form['question'] = array('#markup' => $info_text);
	$form['location_id'] = array('#type' => 'value', '#value' => $location_id);
	$form['location_name'] = array('#type' => 'value', '#value' => $location->location_name);
	$form['location_code'] = array('#type' => 'value', '#value' => $location->location_code);
	$form['submit'] = array('#type' => 'submit', '#value' => 'Remove');
	$form['cancel'] = array('#type' => 'submit', '#value' => 'Cancel');
	return $form;
}

function eaton_backbone_admin_eaton_locations_form_delete_submit($form, &$form_state) {
	switch ($form_state['triggering_element']['#value']) {
	case 'Remove':
		$query = db_delete('eaton_backbone_eaton_locations')->condition('location_id', $form_state['values']['location_id'])->execute();
		$msg = t('Eaton Location <em>@name (@code)</em> has been removed from the system', array('@name' => $form_state['values']['location_name'], '@code' => $form_state['values']['location_code']));
		drupal_set_message($msg);
		break;
	case 'Cancel':
		$form_state['redirect'] = 'admin/config/eaton/locations/locations';
		break;
	}
}

function eaton_backbone_admin_eaton_locations_badging_form() {
	$form = array();
	$form['default'] = array(
		'#type' => 'fieldset',
		'#title' => t('Default Settings'),
		'#collapsible' => false,
		'#collapsed' => false,
	);
	$form['default']['clear_cache'] = array(
		'#type' => 'submit',
		'#value' => t('Clear Cached Badges'),
		'#suffix' => '<div class="description">NOTE: <em>When changing any setting here, clear the badge cache in order for the new settings to be applied and badges rebuilt.</em></div>',
	);
	$form['default']['text_limit'] = array(
		'#type' => 'textfield',
		'#title' => t('Maximum Length of Text on Badge'),
		'#description' => t('Any string longer than this many characters will be trimmed.'),
		'#default_value' => variable_get('eaton_backbone_badging_text_limit', 30),
	);
	$file_fields = array('badge_front', 'badge_back');
	foreach ($file_fields as $field_name) {
		$badge_file = variable_get('eaton_backbone_defaults_' . $field_name, '');
		$side = explode('_', $field_name);
		$form['default'][$field_name] = array(
			'#type' => 'managed_file',
			'#title' => t(ucwords($side[1]) . ' of Badge'),
			'#description' => t('This will get used when a location does not have a badge ' . $side[1] . ' uploaded and creates badges.'),
			'#upload_location' => 'public://eaton_backbone/badging/defaults',
			'#upload_validators' => array(
				'file_validate_extensions' => array('png jpg jpeg'),
			),
			'#default_value' => $badge_file,
		);
		if ($badge_file) {
			// Show preview
			$file = file_load($badge_file);
			$form['default'][$field_name . '_preview'] = array(
				'#prefix' => '<h2>Preview</h2>',
				'#markup' => theme_image(array('path' => file_create_url($file->uri), 'width' => '', 'height' => '', 'alt' => 'Badge ' . $side[1] . ' Preview', 'attributes' => array())),
			);
		}
	}
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Configuration'),
	);
	return $form;
}

function eaton_backbone_admin_eaton_locations_badging_form_validate($form, &$form_state) {
	$values = $form_state['values'];
	if (isset($values['text_limit']) && !empty($values['text_limit'])) {
		if (!is_numeric($values['text_limit'])) {
			form_set_error('text_limit', 'Text limit must be a number.');
		}
	}
}

function eaton_backbone_admin_eaton_locations_badging_form_submit($form, &$form_state) {
	switch ($form_state['triggering_element']['#value']) {
	case 'Save Configuration':
		$send_msg = false;
		$values = $form_state['values'];
		$file_fields = array('badge_back', 'badge_front');
		foreach ($file_fields as $field_name) {
			if (isset($values[$field_name]) && !empty($values[$field_name])) {
				variable_set('eaton_backbone_defaults_' . $field_name, $values[$field_name]);
				$file = file_load($values[$field_name]);
				$file->status = FILE_STATUS_PERMANENT;
				file_save($file);
				file_usage_add($file, 'eaton_backbone', 'defaults', 0);
				$send_msg = true;
			} else if (isset($values[$field_name]) && empty($values[$field_name])) {
				variable_set('eaton_backbone_defaults_' . $field_name, $values[$field_name]);
			}
		}
		if (isset($values['text_limit']) && !empty($values['text_limit'])) {
			variable_set('eaton_backbone_badging_text_limit', $values['text_limit']);
			$send_msg = true;
		}
		if ($send_msg) {
			drupal_set_message('Your changes have been saved successfully');
		}
		break;
	case 'Clear Cached Badges':
		eaton_backbone_clear_cached_badges();
		drupal_set_message("All cached badges have been deleted from the system.  Next time any badge is viewed, it will be rebuilt.");
		break;
	}
}

function eaton_backbone_email_templates_admin_form() {
	$form['templates'] = array(
		'#type' => 'fieldset',
		'#title' => 'Eaton Email Templates',
		'#description' => t('Edit the default text that is placed in approval and denied emails.  <strong>HTML IS ALLOWED.</strong>'),
		'#collapsible' => false,
		'#collapsed' => false,
	);
	$replacements = _eaton_email_template_variables();
	$replacements_help = '<ul>';
	foreach ($replacements as $searchFor => $replaceWith) {
		if (function_exists($replaceWith)) {
			$replacements_help .= '<li>' . $searchFor . ' will be replaced with data from node of same name.</li>';
		} else {
			$replacements_help .= '<li>' . $searchFor . ' will be replaced with ' . $replaceWith . '</li>';
		}
	}
	$replacements_help .= '</ul>';
	$form['templates']['replacements_help'] = array(
		'#markup' => $replacements_help,
		'#prefix' => '<div class="description"><h4>Replacements are available:</h4>',
		'#suffix' => '</div>',
	);
	$form['templates']['eaton_email_template_approved'] = array(
		'#type' => 'textarea',
		'#title' => 'Event Approved',
		'#description' => t('Occurs when an event is approved by the coordinator.'),
		'#default_value' => variable_get('eaton_email_template_approved', ''),
	);
	$form['templates']['eaton_email_template_denied'] = array(
		'#type' => 'textarea',
		'#title' => 'Event Denied',
		'#description' => t('Occurs when an event is denied by the coordinator.'),
		'#default_value' => variable_get('eaton_email_template_denied', ''),
	);
	$form['templates']['eaton_email_template_attendee_registration'] = array(
		'#type' => 'textarea',
		'#title' => 'Attendee Registration',
		'#description' => t('Occurs when an attendee is registered for an event.'),
		'#default_value' => variable_get('eaton_email_template_attendee_registration', ''),
	);
	$form['templates']['eaton_email_template_suggested_date_accepted'] = array(
		'#type' => 'textarea',
		'#title' => 'Suggested Date Accepted',
		'#description' => t('Occurs when an initiator accepts a suggested date from a denial.'),
		'#default_value' => variable_get('eaton_email_template_suggested_date_accepted', ''),
	);
	return system_settings_form($form);
}

function eaton_backbone_admin_eaton_locations_settings() {
	$form['eaton_backbone_eaton_locations_timezones'] = array(
		'#title' => t('Enabled Timezones'),
		'#type' => 'select',
		'#options' => eaton_backbone_timezone_identifiers_list(),
		'#default_value' => variable_get('eaton_backbone_eaton_locations_timezones', ''),
		'#multiple' => true,
		'#size' => 20,
		'#required' => true,
		'#description' => t('Select the timezones that will be available for locations to choose.'),
	);
	return system_settings_form($form);
}

// function eaton_backbone_admin_settings_form() {
//     $form['eaton_backbone_settings_event_approval'] = [
//         '#type' => 'select',
//         '#title' => 'Events require approval',
//         '#options' => ['No', 'Yes']
//     ];
//     return system_settings_form($form);
// }