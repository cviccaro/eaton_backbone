<?php
/**
 * Event Requests page
 *
 * This page lists all event requests, approved or not.
 * The page is customized differently for initiators and coordinators.
 */
function eaton_backbone_event_requests_page() {
	global $user;
	drupal_add_library('system', 'jquery.cookie');
	$is_coordinator = _eaton_backbone_is_coordinator($user);

	// Add date popup assets
	date_popup_add();
	// Add backbone.js to page
	drupal_add_library('backbone_services', 'drupalbackbone-services');
	// Add chosen
	drupal_add_js(libraries_get_path('chosen') . '/chosen.jquery.min.js');
	drupal_add_css(libraries_get_path('chosen') . '/chosen.min.css');

	// Add templates to page.
	$app_template = 'eaton_backbone_event_request_coordinator_app_template';
	if ($is_coordinator) {
		backbone_add_template('eaton-backbone-event-request-coordinator-node-template', theme('eaton_backbone_event_request_coordinator_node_template'));
	} else {
		backbone_add_template('eaton-backbone-event-request-initiator-node-template', theme('eaton_backbone_event_request_initiator_node_template'));
		$app_template = 'eaton_backbone_event_request_initiator_app_template';
	}

	// Add app js.
	eaton_backbone_add_timezone_js();
	drupal_add_js(drupal_get_path('module', 'eaton_backbone') . '/js/event_requests.js');

	// Gather settings to add to Drupal JS settings
	$options = array();

	// Gather field allowed values for use in app when preparing to save (required because of services finnicky-ness)
	$lists = array('field_visitor_type', 'field_request_status', 'field_customer_boolean', 'field_event_visibility');
	foreach ($lists as $field_name) {
		$field = field_info_field($field_name);
		$options[$field_name] = $field['settings']['allowed_values'];
	}
	$locations = eaton_backbone_eaton_locations();
	foreach ($locations as $location) {
		$options['field_event_location'] = eaton_backbone_eaton_locations_options();
	}
	// Gather users information for coordinator usage
	$users = array();
	if ($is_coordinator) {
		// gather list of usernames and full names => uids
		$query = db_select('users', 'u');
		$query->join('field_data_field_first_name', 'fn', 'fn.entity_id = u.uid');
		$query->join('field_data_field_last_name', 'ln', 'ln.entity_id = u.uid');
		$query->fields('u', array('uid', 'name'))->condition('u.uid', 0, '!=');
		$query->fields('fn', array('field_first_name_value'));
		$query->fields('ln', array('field_last_name_value'));
		$users = $query->execute()->fetchAllAssoc('uid');
	}

	// Gather receipts for linking in the table
	$receipts = $is_coordinator ? _eaton_backbone_event_request_receipts() : _eaton_backbone_event_request_receipts($user->uid);
	$emails = $is_coordinator ? _eaton_backbone_event_request_email_receipts() : _eaton_backbone_event_request_email_receipts($user->uid);

	// Get current user to set the default (eaton) location in the JS
	$current_user = user_load($user->uid);
	$default_location = '';
	if (isset($current_user->field_eaton_location) && !empty($current_user->field_eaton_location)) {
		$default_location = $current_user->field_eaton_location[LANGUAGE_NONE][0]['value'];
	}

	// Get initial batch of events
	//$events = array_values(_eaton_backbone_events());

	// Add JS settings to page
	drupal_add_js(array(
		'event_requests' => array(
			//'events' => $events,
			'is_coordinator' => $is_coordinator,
			'options' => $options,
			'default_location' => $default_location,
			'users' => $users,
			'receipts' => $receipts,
			'emails' => $emails,
			'js_timeout' => 10000,
		),
	), 'setting');

	// Return a container div for the app to anchor itself to.
	return theme($app_template, array('timeout' => 10000));
}

// Title callback
function eaton_backbone_event_requests_page_title() {
	global $user;
	return _eaton_backbone_is_coordinator($user) ? 'Events' : 'My Events';
}

// Add Timezone JS
function eaton_backbone_add_timezone_js() {
	$date_default_timezone = variable_get('date_default_timezone');
	$dateTime = new DateTime();
	$dateTime->setTimeZone(new DateTimeZone($date_default_timezone));
	$timezone_db = $dateTime->format('T');
	drupal_add_js(array('eaton_backbone' => array('timezone_db' => $timezone_db)), 'setting');
}

/**
 * Event Request Receipt Page
 */
function eaton_backbone_event_request_receipt_page($receipt_id) {
	global $user;
	$nid = _eaton_backbone_decrypt_receipt_id($receipt_id);
	if ($nid && $node = node_load($nid)) {
		if ($node->type == 'event_request') {
			return theme('eaton_backbone_event_request_receipt', array('node' => $node));
		}
	} else {
		drupal_not_found();
	}
}

// Title callback
function eaton_backbone_event_request_receipt_page_title($receipt_id) {
	$nid = _eaton_backbone_decrypt_receipt_id($receipt_id);
	if ($nid) {
		$title = 'Event Request Receipt';
		$node = db_select('node', 'n')->fields('n', array('title', 'uid'))->condition('nid', $nid)->execute()->fetchObject();
		$user = db_select('users', 'u')->fields('u', array('name'))->condition('uid', $node->uid)->execute()->fetchObject();
		$title = t('Event summary for @user\'s !title Event', array('@user' => $user->name, '!title' => $node->title));
		drupal_set_title($title);
		return $title;
	}
}

// Access callback
function eaton_backbone_event_request_receipt_page_access($receipt_id) {
	global $user;
	// admin/coordinator has access to all
	if (_eaton_backbone_is_coordinator($user)) {
		return true;
	} else {
		// only allow node author
		$nid = _eaton_backbone_decrypt_receipt_id($receipt_id);
		if ($nid && $node = node_load($nid)) {
			if ($user->uid == $node->uid) {
				return true;
			}
		}
	}
	return false;
}

/**
 * Approved Event Preregistration Page
 */
function eaton_backbone_approved_event_preregistration_page($node) {
	if ($node && $node->type == 'event_request' && $node->field_request_status[LANGUAGE_NONE][0]['value'] == 'approved') {
		$build = array();
		$build['eaton_backbone_event_details'] = array('#theme' => 'eaton_backbone_event_details', 'node' => $node);

		// Build forms one at a time, check for errors, show messages if there are errors.
		$errors = false;

		// Upload Form
		$upload_form = drupal_get_form('eaton_backbone_approved_event_preregistration_upload_form', $node);
		if (form_get_errors() && !$errors) {
			$build['messages'] = array('#markup' => theme_status_messages(array('display' => 'error'))); // Display error messages
			$errors = true;
		}
		$build['eaton_backbone_approved_event_preregistration_upload_form'] = array('form' => $upload_form);

		// Add Existing Registrant Form
		// $add_existing_form = drupal_get_form('eaton_backbone_approved_event_preregistration_add_existing_registrant_form', $node);
		// if (form_get_errors() && !$errors) {
		//     $build['messages'] = array('#markup' => theme_status_messages(array('display' => 'error'))); // Display error messages
		// }
		// $build['eaton_backbone_approved_event_preregistration_add_existing_registrant_form'] = array('form' => $add_existing_form);

		// Add New Registrant Form
		$registrant_form = drupal_get_form('eaton_backbone_event_registrant_form', $node);
		$registrant_form['heading'] = array(
			'#markup' => t('Add New Registrant to this Event'),
			'#prefix' => '<h2>',
			'#suffix' => '</h2>',
			'#weight' => -100,
		);
		if (form_get_errors() && !$errors) {
			$build['messages'] = array('#markup' => theme_status_messages(array('display' => 'error'))); // Display error messages
		}
		$build['eaton_backbone_event_registrant_form'] = array('form' => $registrant_form);

		// Edit form used in dialog
		$build['eaton_backbone_event_registrant_edit_form'] = array('form' => drupal_get_form('eaton_backbone_event_registrant_edit_form', $node));

		// Backbone Attendee table
		$build['eaton_backbone_event_attendees_app'] = array('#theme' => 'eaton_backbone_event_attendees_app_template', '#nid' => $node->nid);

		// Add JS and backbone
		drupal_add_library('backbone_services', 'drupalbackbone-services');
		backbone_add_template('eaton-backbone-event-attendees-template', theme('eaton_backbone_event_attendees_attendee_template'));
		drupal_add_js(drupal_get_path('module', 'eaton_backbone') . '/js/event_preregistration.js');
		drupal_add_js(drupal_get_path('module', 'eaton_backbone') . '/js/badge-links.js');

		drupal_add_js(array('event_preregistration' => array('node' => $node)), 'setting');

		$_SESSION['eaton_event_preregistration_id'] = $node->nid;
		return $build;
	} else {
		drupal_not_found();
	}
}

// Access callback
function eaton_backbone_approved_event_preregistration_access($node) {
	global $user;
	// admin/coordinator has access to all
	if (_eaton_backbone_is_coordinator($user)) {
		return true;
	} else {
		// else only allow node author
		if ($node->type == 'event_request' && $user->uid == $node->uid) {
			return true;
		}
	}
	return false;
}

// Title callback
function eaton_backbone_approved_event_preregistration_page_title($node) {
	return t('!title Preregistration Form', array('!title' => $node->title));
}

/**
 * Public Registration Page
 */
function eaton_backbone_public_event_registration_page($node) {
	// Only show registration page for approved event requests.
	//$is_public = _eaton_backbone_event_is_public($node);
	if ($node->type == 'event_request') {
// && _eaton_backbone_event_is_approved($node) && $is_public) {
		//if ($node->type == 'event_request' && _eaton_backbone_event_is_approved($node)) {
		$build = array();
		//if ($is_public) {
		$build['event_image'] = [
			'#theme' => 'image',
			'#path' => path_to_theme() . '/img/private-reg-splash.jpg',
			'#attributes' => [
				'class' => 'splash-image',
			],
		];
		$build['event_details_invite'] = array('#theme' => 'eaton_backbone_event_details_invite', 'node' => $node);
		$registrant_form = drupal_get_form('eaton_backbone_event_registrant_form', $node);
		$registrant_form['nid']['#value'] = $node->nid;
		$errors = form_get_errors();
		$build['messages'] = array('#markup' => theme_status_messages(array('display' => 'error')));
		$build['registrant_form'] = array('form' => $registrant_form);
		// }
		// else {
		//  $build['event_details'] = array('#markup' => t('<div style="margin:16px">Sorry, this event is private and is closed to public registration.</div>'));
		// }
		return $build;
	} else {
		drupal_not_found();
	}
}

// Title callback
function eaton_backbone_public_event_registration_page_title($node) {
	return t('Register for Event !title', ['!title' => $node->title]);
}

/**
 * Public Registration Thanks/Receipt Page
 */
function eaton_backbone_public_registration_thanks_page($receipt_id) {
	$components = _eaton_backbone_decrypt_registration_id($receipt_id);
	if ($components && is_array($components) && count($components) >= 2) {
		$event_id = $components['event_id'];
		$attendee_id = $components['attendee_id'];
		if (_attendeeresource_attendee_id_exists_event($attendee_id, $event_id)) {
			return theme('eaton_backbone_public_registration_thanks', $components);
		}
	} else {
		drupal_not_found();
	}
}

// Title callback
function eaton_backbone_public_registration_thanks_page_title($receipt_id) {
	$components = _eaton_backbone_decrypt_registration_id($receipt_id);
	if ($components && is_array($components)) {
		$event_id = $components['event_id'];
		$attendee_id = $components['attendee_id'];
		// If the title gets set when the page function returns a drupal_not_found(), the title still gets used..
		// so this check is unfortunately necessary here.
		if (_attendeeresource_attendee_id_exists_event($attendee_id, $event_id)) {
			$node = db_select('node', 'n')->fields('n', array('title', 'uid'))->condition('nid', $components['event_id'])->execute()->fetchObject();
			$attendee = _attendeeresource_get_attendee($components['attendee_id']);
			$title = t('Thank you, @attendee, for registering for the "@event_name" event!', array('@attendee' => $attendee->first_name . ' ' . $attendee->last_name, '@event_name' => $node->title));
			drupal_set_title($title);
			return $title;
		}
	}
}

/**
 * Email pages
 */
function eaton_backbone_email_page($email_id) {
	// registration emails decrypt using this function, request status emails will not
	$components = _eaton_backbone_decrypt_registration_id($email_id);
	if ($components) {
		return eaton_backbone_public_registration_email_page($email_id);
	} else {
		return eaton_backbone_request_status_email_page($email_id);
	}
}

function eaton_backbone_public_registration_email_page($email_id) {
	global $user;
	$components = _eaton_backbone_decrypt_registration_id($email_id);
	if ($components) {
		if ($node = node_load($components['event_id'])) {
			if ($attendee = _attendeeresource_get_attendee($components['attendee_id'])) {
				$vars = array(
					'node' => $node,
					'attendee' => $attendee,
					'receipt_id' => $email_id,
				);
				if (isset($components['type']) && $components['type'] == 'pre') {
					$preregistration_defaults = _eaton_backbone_preregistration_email_defaults();
					$vars['subject'] = $preregistration_defaults['subject'];
					$vars['message'] = $preregistration_defaults['message'];
				} else {
					$public_defaults = _eaton_backbone_public_registration_email_defaults();
					$vars['subject'] = $public_defaults['subject'];
					$vars['message'] = $public_defaults['message'];
				}
				print '<style type="text/css">.email-link { display: none; }</style>' . theme('eaton_backbone_registration_email', $vars);
			}
		} else {
			drupal_not_found();
			exit();
		}
	} else {
		drupal_not_found();
		exit();
	}
}

function eaton_backbone_request_status_email_page($email_id) {
	global $user;
	$nid = _eaton_backbone_decrypt_email_id($email_id);
	$node = node_load($nid);
	$not_found = true;
	if ($node && $node->type == 'event_request') {
		$wrapper = entity_metadata_wrapper('node', $node);
		$status = $wrapper->field_request_status->value();
		print '<style type="text/css">.email-link { display: none; }</style>' . theme('eaton_backbone_event_email', array('node' => $node));
		$not_found = false;
		// if ($status == 'approved') {
		//     $not_found = false;
		//     print '<style type="text/css">.email-link { display: none; }</style>' . theme('eaton_backbone_approved_email', array('node' => $node));
		// } elseif ($status == 'denied') {
		//     $not_found = false;
		//     print '<style type="text/css">.email-link { display: none; }</style>' . theme('eaton_backbone_denied_email', array('node' => $node));
		// }
	}
	if ($not_found) {
		drupal_not_found();
		exit();
	}
}

/**
 * CSV Template page
 */
function eaton_backbone_csv_template_page() {
	drupal_add_http_header('Content-type', 'text/csv');
	$template = _attendeeresource_generate_template_from_schema('attendeeresource_attendees');

	$example = array();
	foreach ($template as $key) {
		switch ($key) {
		case 'country':
			$example[] = '"US"';
			break;
		case 'state':
			$example[] = '"PA"';
			break;
		case 'email':
			$example[] = '"example@email.com"';
			break;
		default:
			$example[] = '"Example ' . ucwords(str_replace('_', ' ', $key)) . '"';
		}
	}
	print implode(',', $template) . "\r\n";
	print implode(',', $example);
	exit();
}

function eaton_backbone_xlsx_template_page() {
	// Grab the template, and output it to the browser
	$file = eaton_backbone_export_excel_template();
	$headers = file_get_content_headers($file);
	file_transfer($file->uri, $headers);
}

/**
 * ICS File Page (literally delivers an ICS file)
 */
function eaton_backbone_ics_page($node) {
	if ($node && _eaton_backbone_event_is_approved($node)) {
		$location_code = $node->field_event_location[LANGUAGE_NONE][0]['value'];
		$datestart = !empty($node->field_approved_date) ? strtotime($node->field_approved_date[LANGUAGE_NONE][0]['value']) : '';
		$dateend = !empty($node->field_approved_date[LANGUAGE_NONE][0]['value2']) ? strtotime($node->field_approved_date[LANGUAGE_NONE][0]['value2']) : $datestart;
		$location_options = _eaton_backbone_options_office_locations();
		$title = implode('_', explode(' ', _eaton_backbone_clean_text($node->title)));
		$filename = 'EatonEvent_' . $title . '.ics';
		$description = $node->title;
		$approved_on = $node->changed_local;
		$description .= '\nApproved on ' . $approved_on;

		$location = $location_options[$location_code];
		$location_address = eaton_backbone_eaton_location_address($location_code, false);
		$eaton_location = eaton_backbone_eaton_location_by_code($location_code);
		if ($location_address) {
			$location .= '\n' . $location_address;
		}
		$contact = '';
		if (!empty($eaton_location->location_contact)) {
			$contact .= $eaton_location->location_contact;
		}
		if (!empty($eaton_location->location_contact_title)) {
			$spacer = !empty($contact) ? ', ' : '';
			$contact .= $spacer . $eaton_location->location_contact_title;
		}
		if (!empty($eaton_location->location_phone)) {
			$contact .= '\n' . $eaton_location->location_phone;
		}
		if (!empty($eaton_location->location_email)) {
			$contact .= '\n' . $eaton_location->location_email;
		}

		if (!empty($contact)) {
			$location .= '\n\nContact:\n' . $contact;
		}

		$variables = array(
			'event_id' => $node->nid,
			'location' => $location,
			'datestart' => $datestart,
			'dateend' => $dateend,
			'description' => $description,
			'summary' => $node->title,
			'uri' => url('ics/' . $node->nid, array('absolute' => true)),
			'filename' => $filename,
			'tzid' => eaton_backbone_timezone_for_location($location_code),
		);
		print theme('eaton_backbone_ics', $variables);
		exit();
	} else {
		return drupal_not_found();
	}
}

function _eaton_backbone_clean_text($string) {
	$string = str_replace(' ', '-', $string); // Replaces all spaces with hyphens.
	return preg_replace('/[^A-Za-z0-9\-]/', '', $string); // Removes special chars.
}

function eaton_backbone_csv_template_help() {
	$build = array();
	$template = _attendeeresource_generate_template_from_schema('attendeeresource_attendees');

	$build['template'] = array(
		'#markup' => '<p>Your CSV should have the following columns in the following order: <em>' . implode(', ', array_values($template)) . '</em></p>',
	);
	// $visitor_types = _eaton_backbone_visitor_types();
	// $build['visitor_types'] = array(
	// '#markup' => '<h2>Job Functions</h2><h4>Possible values for column <em>Job Function</em></h4><ul class="list">',
	// );
	// foreach ($visitor_types as $key => $type) {
	//     if (!empty($key)) {
	//         $build['visitor_types']['#markup'] .= '<li>' . $key . '</li>';
	//     }
	// }
	// $build['visitor_types']['#markup'] .= '</ul>';

	$market_focus = _eaton_backbone_market_focus();
	$build['market_focus'] = array(
		'#markup' => '<h2>Industries</h2><h4>Possible values for column <em>Industry</em></h4><ul class="list">',
	);
	foreach ($market_focus as $key => $type) {
		if (!empty($key)) {
			$build['market_focus']['#markup'] .= '<li>' . $key . '</li>';
		}
	}
	$build['market_focus']['#markup'] .= '</ul>';

	return $build;
}

function eaton_backbone_xls_template_help() {
	$build = array();
	$template = _attendeeresource_generate_template_from_schema('attendeeresource_attendees');

	$build['template'] = array(
		'#markup' => '<p>Your XLS should have the following columns in the following order: <em>' . implode(', ', array_values($template)) . '</em></p>',
	);
	$template_link = l('the template', 'templates/attendees.xlsx');
	$build['template']['#markup'] .= sprintf('<p>You can download the %s which already has these columns set up and includes drop-downs for the various choices below.</p>', $template_link);
	$build['template']['#markup'] .= '<p><strong>IMPORTANT:</strong> The column titles must exist as the first row in the document.</p>';
	// $visitor_types = _eaton_backbone_visitor_types();
	// $build['visitor_types'] = array(
	// '#markup' => '<h2>Job Functions</h2><h4>Possible values for column <em>Job Function</em></h4><ul class="list">',
	// );
	// foreach ($visitor_types as $key => $type) {
	//     if (!empty($key)) {
	//         $build['visitor_types']['#markup'] .= '<li>' . $key . '</li>';
	//     }
	// }
	// $build['visitor_types']['#markup'] .= '</ul>';

	$market_focus = _eaton_backbone_market_focus();
	$build['market_focus'] = array(
		'#markup' => '<h2>Industries</h2><h4>Possible values for column <em>Industry</em></h4><ul class="list">',
	);
	foreach ($market_focus as $key => $type) {
		if (!empty($key)) {
			$build['market_focus']['#markup'] .= '<li>' . $key . '</li>';
		}
	}
	$build['market_focus']['#markup'] .= '</ul>';

	return $build;
}

/**
 * Deliver one badge
 */
function eaton_backbone_deliver_badge($node, $attendee_id, $print_pdf = true, $side = 'front') {
	global $base_url;

	$css_path = drupal_get_path('theme', 'eaton') . '/stylesheets/badge.css';
	$qr_width = $qr_height = 55;
	$theme_hook_front = 'eaton_backbone_badge';

	// Get location
	$wrapper = entity_metadata_wrapper('node', $node);
	$location_code = $wrapper->field_event_location->value();
	$location = eaton_backbone_eaton_location_by_code($location_code);

	// Alter attributes if size is set to large
	if ($location->qr_size == 'large') {
		$qr_width = $qr_height = 100;
		$css_path = drupal_get_path('theme', 'eaton') . '/stylesheets/badge-large-qr.css';
		$theme_hook_front = 'eaton_backbone_badge_large_qr';
	}

	if (isset($_GET['pdf']) && $_GET['pdf']) {
		$print_pdf = true;
	} elseif (isset($_GET['html'])) {
		$print_pdf = false;
	}

	$attendee = _attendeeresource_get_attendee($attendee_id);
	if ($attendee) {
		$badge_front = eaton_backbone_badge_front_for_event($node);
		$output = '<style type="text/css">' . file_get_contents($css_path) . '</style>';
		$output .= '<link rel="stylesheet" href="' . $base_url . '/' . $css_path . '" />';
		if ($side == 'front') {
			if ($file = _eaton_backbone_generate_qr_code($node->nid, $attendee)) {
				$output .= theme(
					$theme_hook_front,
					array(
						'node' => $node,
						'image_height' => $qr_height,
						'image_width' => $qr_width,
						'attendee' => $attendee,
						'file' => $file,
						'wrapper_attributes' => array(
							'class' => array('badge-table', 'badge'),
						),
						'background' => $badge_front,
					)
				);
				if ($print_pdf) {
					$pdf = new HTML2PDF('P', array(86, 54), 'en', true, 'UTF-8', array(0, 0, 0, 0));
					$pdf->WriteHTML($output);
					$pdf->Output('badge.pdf');
				} else {
					return $output;
				}
			}
		} else {
			$badge_back = eaton_backbone_badge_back($node);
			if ($badge_back) {
				$output .= theme(
					'eaton_backbone_badge_back',
					array(
						'background' => file_create_url($badge_back->uri),
						'node' => $node,
					)
				);
				if ($print_pdf) {
					$pdf = new HTML2PDF('P', array(86, 54), 'en', true, 'UTF-8', array(0, 0, 0, 0));
					$pdf->WriteHTML($output);
					$pdf->Output('badge-back.pdf');
				} else {
					return $output;
				}
			} else {
				drupal_not_found();
			}
		}
	} else {
		drupal_not_found();
	}
}

function eaton_backbone_deliver_badge_access($node, $attendee_id) {
	return true;
}

/**
 * Function eaton_backbone_deliver_event_badges().
 * Delivers all badges for an event
 */
function eaton_backbone_deliver_event_badges(stdClass $node, $print_pdf = true, $return_output = false, $side = 'front') {
	drupal_set_time_limit(300);
	if (isset($_GET['pdf']) && $_GET['pdf']) {
		$print_pdf = true;
	} elseif (isset($_GET['html']) && $_GET['html']) {
		$print_pdf = false;
	}

	//Get stored file first.
	if (!$return_output && $print_pdf && !isset($_GET['rebuild'])) {
		if ($file = _eaton_backbone_stored_event_badges($node, $side)) {
			watchdog(__FUNCTION__, 'PDF for event ' . $node->nid . ' requested.  Delivering stored file stored as fid ' . $file->fid, null, WATCHDOG_INFO);
			file_transfer($file->uri, array('Content-Type' => 'application/pdf'));
			return '';
		}
	}

	global $base_url;

	$attendees = _attendeeresource_event_attendees($node->nid);

	$output = array();

	if ($side == 'front') {
		foreach ($attendees as $attendee) {
			$output[] = eaton_backbone_deliver_badge($node, $attendee->attendee_id, false, $side);
		}
	} else {
		$badge_back = eaton_backbone_badge_back($node);
		if ($badge_back) {
			$css_path = drupal_get_path('theme', 'eaton') . '/stylesheets/badge.css';
			$output[] = '<style type="text/css">' . file_get_contents($css_path) . '</style>';
			$output[] = '<link rel="stylesheet" href="' . $base_url . '/' . $css_path . '" />';
			foreach ($attendees as $attendee) {
				$output[] = theme(
					'eaton_backbone_badge_back',
					array(
						'background' => file_create_url($badge_back->uri),
						'node' => $node,
					)
				);
			}
		}
	}
	if (!empty($output)) {
		$output = implode("\r\n", $output);
		if ($print_pdf) {
			$pdf = new HTML2PDF('P', array(86, 54), 'en', true, 'UTF-8', array(0, 0, 0, 0));
			//$pdf->setModeDebug();
			$pdf->WriteHTML($output);

			// Store PDF first before outputting it
			$pdf_contents = $pdf->Output('', 'S');
			$fileURI = _eaton_backbone_event_badges_uri($node, $side);
			eaton_backbone_save_pdf($fileURI, $pdf_contents, $side, $node->nid);

			// Now output
			$pdf->Output();
		} else {
			if ($return_output) {
				return $output;
			} else {
				print $output;
				exit();
			}
		}
	} else {
		drupal_not_found();
	}
}

// Access callback
function eaton_backbone_deliver_event_badges_access($node) {
	global $user;
	return _eaton_backbone_is_coordinator($user) || _eaton_backbone_is_initiator($user);
}

// Back of Badge file
function eaton_backbone_badge_back($node) {
	$wrapper = entity_metadata_wrapper('node', $node);
	$location_code = $wrapper->field_event_location->value();
	$location = eaton_backbone_eaton_location_by_code($location_code);
	if ($location) {
		if (isset($location->badge_back) && !is_null($location->badge_back)) {
			return file_load($location->badge_back);
		} else {
			// return default
			$fid = variable_get('eaton_backbone_defaults_badge_back', '');
			if ($fid) {
				return file_load($fid);
			}
		}
	}
	return false;
}

/**
 * Function _eaton_backbone_generate_qr_code().
 *
 * Uses classes in inc/ to create QR code
 *
 * @param type $event_id
 * @param type $attendee
 * @return $file object or FALSE
 */
function _eaton_backbone_generate_qr_code($event_id, $attendee) {
	// $qr = _eaton_backbone_get_saved_qr_code($event_id, $attendee->attendee_id);
	// if ($qr && (time() - $qr->timestamp < 86400)) {
	//   return $qr;
	// }
	$event_name = db_select('node', 'n')->fields('n', array('title'))->condition('nid', $event_id)->execute()->fetchField();
	if (isset($attendee->events)) {
		unset($attendee->events);
	}
	$sticker = new QrCodeSticker('eaton_event|' . $event_id . '|' . $attendee->attendee_id . '|' . $attendee->email . '|' . $event_name, $attendee->first_name . ' ' . $attendee->last_name . ' (' . $attendee->email . ')');
	$data = $sticker->genBarcode(); // get png data
	if ($data) {
		$target_dir = 'public://event_' . $event_id . '_badges';
		file_prepare_directory($target_dir, FILE_CREATE_DIRECTORY);
		$file = file_save_data($data, $target_dir . '/attendee_' . $attendee->attendee_id . '_badge.png', FILE_EXISTS_REPLACE);

		//file_usage_add($file, 'eaton_backbone', 'attendee', $attendee->attendee_id);
		return $file;
	}
	return false;
}

function _eaton_backbone_get_saved_qr_code($event_id, $attendee_id) {
	$fid = db_select('file_managed', 'f')->fields('f', array('fid'))->condition('uri', 'public://event_' . $event_id . '_badges/attendee_' . $attendee_id . '_badge.png')->execute()->fetchField();
	if ($fid) {
		return file_load($fid);
	}
	return false;
}

/**
 * My Registrations Page
 */
function eaton_backbone_my_registrations_page() {
	global $user;
	$build = array();
	$attendee = _attendeeresource_lookup_attendee($user->mail);
	if ($attendee) {
		$build['attendee_details'] = array('#theme' => 'eaton_backbone_attendee_details', '#attendee' => $attendee);
		$build['my-registrations'] = array('#theme' => 'eaton_backbone_my_registrations', '#attendee' => $attendee);
	} else {
		$build['attendee_details'] = array('#markup' => t('You do not exist as an attendee in the system.  Register for an event as an attendee to view your registrations.'));
	}
	return $build;
}

/**
 * Today's Events and Attendees page
 */
function eaton_backbone_today_page() {
	global $user;
	$build = array();
	$query = new EntityFieldQuery();
	$today_date = date('Y-m-d');
	$today_time = strtotime($today_date . '00:00:00');
	$today = date('Y-m-d\TH:i:s', $today_time);
	$eod_time = strtotime($today_date . ' 23:59:59');
	$eod = date('Y-m-d\TH:i:s', $eod_time);
	$query->entityCondition('entity_type', 'node')
		->entityCondition('bundle', 'event_request')
		->propertyCondition('status', 1)
		->fieldCondition('field_request_status', 'value', 'approved');

	$location_filter = isset($_GET['location']) ? $_GET['location'] : '';

	if ($location_filter != '') {
		$query->fieldCondition('field_event_location', 'value', $location_filter);
	}

	$showing = 0;
	$result = $query->execute();

	$build['eaton_backbone_event_location_filter'] = array('form' => drupal_get_form('eaton_backbone_event_location_filter_form', $location_filter), '#weight' => -50);

	if (!empty($result)) {
		foreach ($result['node'] as $nid => $node_info) {
			$node = node_load($nid);
			$wrapper = entity_metadata_wrapper('node', $node);
			$approved_date = $wrapper->field_approved_date->value();
			$start_time = strtotime($approved_date['value']);
			$end_time = strtotime($approved_date['value2']);
			$show_it = false;
			if ($start_time >= $today_time && $start_time <= $eod_time) {
				$show_it = true;
			} elseif ($start_time <= $today_time && $end_time >= $today_time) {
				$show_it = true;
			}
			if ($show_it && eaton_backbone_user_event_access($nid, $user->uid)) {
				$showing++;
				$build['eaton_backbone_event_' . $nid . '_event_attendees'] = array('#theme' => 'eaton_backbone_event_attendees', '#node' => $node, '#weight' => $showing * 10);
			}
		}
	}
	if ($showing == 0) {
		$build['eaton_backbone_event_attendees_empty'] = array('#markup' => '<h2 class="subtitle">There are no events scheduled for today for this location.</h2>', '#weight' => 0);
		$node = eaton_backbone_next_event($location_filter);
		if ($node) {
			$display = array(
				'label' => 'hidden',
				'type' => 'date_default',
				'settings' => array(
					'format_type' => 'medium',
				),
			);
			$approved_date = strip_tags(render(field_view_field('node', $node, 'field_approved_date', $display)));
			$next_event = '<p>The next event is <em>' . l($node->title, 'event/' . $node->nid) . '</em>, scheduled for ' . $approved_date . '</p>';
			$build['eaton_backbone_event_next'] = array('#markup' => $next_event, '#weight' => 10);
		} else {
			$build['eaton_backbone_event_next'] = array('#markup' => '<p>There are no upcoming events for this location.</p>', '#weight' => 10);
		}
	}
	return $build;
}

function eaton_backbone_event_location_filter_form($form, $form_state, $location_filter = null) {
	$form['location'] = array(
		'#type' => 'select',
		'#title' => t('Filter by Location'),
		'#options' => array(-1 => '-- Any Location --') + eaton_backbone_eaton_locations_options(),
		'#default_value' => !is_null($location_filter) ? $location_filter : '',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Filter'),
	);
	$form['#attributes']['class'][] = 'eaton-location-filter-form';
	return $form;
}

function eaton_backbone_event_location_filter_form_validate($form, &$form_state) {
	$values = $form_state['values'];
	$location = $values['location'];
	if ($location == '') {
		form_set_error('location', 'To filter, please choose a location first.');
	}
}
function eaton_backbone_event_location_filter_form_submit($form, &$form_state) {
	$values = $form_state['values'];
	$location = $values['location'];
	if ($location == -1) {
		drupal_goto(current_path());
	} else {
		drupal_goto(current_path(), array('query' => array('location' => $location)));
	}
}

function eaton_backbone_accept_suggested_date_title($node) {
	return t('Accept Suggested Date for ' . $node->title);
}

function eaton_backbone_accept_suggested_date_page($node) {
	if (_eaton_backbone_event_is_denied($node)) {
		$build = array();
		$build['messages'] = array('#markup' => theme_status_messages(array('display' => 'status'))); // Display status messages
		$display = array(
			'format_type' => 'long',
			'fromto' => 'both',
			'label' => 'above',
			'type' => 'date_default',
			'settings' => array(
				'format_type' => 'medium',
				'fromto' => 'both',
			),
		);
		$field_view = field_view_field('node', $node, 'field_denied_suggested_date', $display);
		$field_view['#label_display'] = 'above';
		$field_view['#view_mode'] = 'default';
		if (_eaton_backbone_denied_event_suggested_date_accepted($node)) {
			$field_view['#title'] = 'Accepted Suggestion Date';
			$description = '';

			if (!isset($_SESSION['eaton_backbone_accept_suggested_nowarn'])) {
				$description .= '<p>You have already accepted the suggested date.  Your event request is undergoing approval again and you will be notified when it is updated.</p>';
			} else {
				unset($_SESSION['eaton_backbone_accept_suggested_nowarn']);
			}
			$description .= render($field_view);
			$build['accept_suggested_date_exists'] = array(
				'#markup' => $description,
			);
		} else {
			$field_view['#title'] = 'Coordinator\'s Suggested Date';
			$description = '<p>Your event request was denied, but a different date was suggested by the coordinator.</p>';

			$description .= render($field_view);
			$description .= '<p>Please click the button below to alert the coordinator that you would like to accept this date, and you will be notified if the event request is then approved.</p>';
			//$description .= '<p>&nbsp;</p>';

			$build['accept_suggested_date_form_description'] = array(
				'#markup' => $description,
			);
			$build['accept_suggested_date_form'] = array('form' => drupal_get_form('eaton_backbone_accept_suggested_date_form', $node->nid));
			//dpm($build);
		}

		return $build;
	} else {
		drupal_not_found();
	}
}

function eaton_backbone_accept_suggested_date_access($node) {
	global $user;
	return $node->uid == $user->uid || _eaton_backbone_is_coordinator($user);
}

function eaton_backbone_accept_suggested_date_form($form, &$form_state, $nid) {
	$form['nid'] = array('#type' => 'value', '#value' => $nid);
	$form['submit'] = array('#type' => 'submit', '#value' => t('Accept Suggested Date'));
	return $form;
}

function eaton_backbone_accept_suggested_date_form_submit($form, $form_state) {
	$node = node_load($form_state['values']['nid']);
	$node_wrapper = entity_metadata_wrapper('node', $node);
	$record = array(
		'event_id' => $node->nid,
		'date' => serialize($node_wrapper->field_denied_suggested_date->value()),
		'timestamp' => time(),
	);
	drupal_write_record('eaton_backbone_accepted_suggestions', $record);
	drupal_set_message('Thank you.  You will be notified shortly if the suggested date is approved.');
	_eaton_backbone_dispatch_suggested_date_accepted_email($node);
	$_SESSION['eaton_backbone_accept_suggested_nowarn'] = true;
}

function eaton_backbone_ajax_edit_event_request($node, $action) {
	switch ($action) {
	case 'approve':
		//this will be the delta of the selected value of field_possible_dates,
		// or, if it equals suggested, it corresponds to the suggested date
		$date_delta = $_REQUEST['date'];
		$node_wrapper = entity_metadata_wrapper('node', $node);
		$node_wrapper->field_request_status = 'approved';
		$date_value = '';
		if ($date_delta == 'suggested') {
			$date_value = $node->field_denied_suggested_date[LANGUAGE_NONE][0];
		} else {
			$date_value = $node->field_possible_dates[LANGUAGE_NONE][$date_delta];
		}
		if (isset($_REQUEST['approve_private']) && $_REQUEST['approve_private']) {
			$node_wrapper->field_event_visibility = 'private';
		}
		// if (!isset($date['value2'])) {
		//   $date['value2'] = $date['value'];
		// }
		// $date['value'] = date("Y-m-d H:i:s",strtotime($date['value']['date'] . ' ' . $date['value']['time']));
		// $date['value2'] = date("Y-m-d H:i:s",strtotime($date['value2']['date'] . ' ' . $date['value2']['time']));
		//
		$node->field_approved_date[LANGUAGE_NONE][0] = (array) $date_value;
		node_save($node);
		$saved = false;
		$wrapper = entity_metadata_wrapper('node', $node);
		$accepted_date = $wrapper->field_approved_date->value();
		if (!empty($accepted_date)) {
			$saved = true;
		}
		//watchdog(__FUNCTION__, 'date came in like this: <pre>' . print_r($date_delta, TRUE) . '.  outputted to ' . print_r($date_value, TRUE) . '</pre>',NULL,WATCHDOG_INFO);
		///    $saved = FALSE;
		return drupal_json_output(array('saved' => $saved, 'node' => $node));
		break;
	case 'deny':
		$date = $_REQUEST['suggested_date'];
		$message = $_REQUEST['suggested_message'];

		// $timezone = new DateTimeZone(date_default_timezone_get());

		// $now = new DateTime(time(), $timezone);

		$timezone = new DateTimeZone(date_default_timezone_get());
		$now = new DateTime();
		$now->setTimeZone($timezone);
		$now->setTimeStamp(time());

		$node->field_denied_on_date = array();
		$node->field_denied_on_date[LANGUAGE_NONE][0]['value'] = $now->format('Y-m-d H:i:s');

		//$all_day = isset($date['all_day']) && $date['all_day'] == 1;
		//$show_todate = isset($date['show_todate']) && $date['show_todate'] == 1;
		$all_day = false;
		$show_todate = true;

		if ($all_day) {
			$date['value']['time'] = '00:00:00';
		}

		$wrapper = entity_metadata_wrapper('node', $node);
		$wrapper->field_request_status = 'denied';
		$wrapper->field_denied_message = check_plain($message);

		$node->field_approved_date = array();
		$node->field_denied_suggested_date = array();

		$start_timestamp = strtotime($date['value']['date'] . ' ' . $date['value']['time']);
		$start_date = new DateTime(null, $timezone);
		$start_date->setTimestamp($start_timestamp);

		$node->field_denied_suggested_date[LANGUAGE_NONE][0]['value'] = $start_date->format('Y-m-d H:i:s');

		if (!$all_day && $show_todate) {
			$end_timestamp = strtotime($date['value2']['date'] . ' ' . $date['value2']['time']);
			$end_date = new DateTime(null, $timezone);
			$end_date->setTimestamp($end_timestamp);

			$node->field_denied_suggested_date[LANGUAGE_NONE][0]['value2'] = $end_date->format('Y-m-d H:i:s');
		}
		$saved = false;
		node_save($node);

		$node = node_load($node->nid);
		$wrapper = entity_metadata_wrapper('node', $node);
		$denied_date = $wrapper->field_denied_suggested_date->value();
		$denied_date_formatted = '';
		if (!is_null($denied_date)) {
			$saved = true;
			$denied_date_formatted = t('@start to @end', array('@start' => $denied_date['value_formatted'], '@end' => $denied_date['value2_formatted']));
		}
		return drupal_json_output(array('saved' => $saved, 'node' => $node, 'denied_date_formatted' => $denied_date_formatted));
		break;
	}
}

function eaton_backbone_ajax_edit_event_request_access($node, $action) {
	global $user;
	return _eaton_backbone_is_coordinator($user);
}

function eaton_backbone_attendee_form_options_json($type) {
	$options = array();
	switch ($type) {
	case 'market_focus':
		$options = _eaton_backbone_market_focus();
		break;
	case 'visitor_type':
		$options = _eaton_backbone_visitor_types();
		break;
	}
	return drupal_json_output(array('options' => $options));
}

function eaton_backbone_badge_front_for_event($node) {
	$wrapper = entity_metadata_wrapper('node', $node);
	$location_code = $wrapper->field_event_location->value();
	$location = eaton_backbone_eaton_location_by_code($location_code);
	$file = false;
	if (!empty($location->badge_front)) {
		$file = file_load($location->badge_front);
	} else {
		$default_fid = variable_get('eaton_backbone_defaults_badge_front', '');
		if ($default_fid) {
			$file = file_load($default_fid);
		}
	}
	if ($file) {
		return file_create_url($file->uri);
	}
	return false;
}
