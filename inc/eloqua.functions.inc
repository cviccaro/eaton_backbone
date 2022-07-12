<?php
/**
 * Eaton Eloqua connector
 * --
 * Created in February 2016
 * 
 * Utilizing API 1.0 endpoint functions from Oracle/Eloqua
 */

define('EATON_ELOQUA_FORM_ID', 1795);


/**
 * Authorize with Eloqua and receive an access token
 * @param  {string} $refresh_token 
 * @return HTTP response object
 * 
 * @link http://docs.oracle.com/cloud/latest/marketingcs_gs/OMCAB/index.html#Developers/GettingStarted/Authentication/authenticate-using-basic-auth.htm
 * @link http://docs.oracle.com/cloud/latest/marketingcs_gs/OMCAB/index.html#Developers/GettingStarted/Authentication/authenticate-using-oauth.htm
 */
function eaton_eloqua_authorize($refresh_token = NULL) {
	$url = 'https://login.eloqua.com/auth/oauth2/token';

	// Combine the client ID and secret and encode with base64
	$client_id = variable_get('eaton_eloqua_client_id');
	$client_secret = variable_get('eaton_eloqua_client_secret');
	$auth_string = base64_encode($client_id . ':' . $client_secret);

	$username = variable_get('eaton_eloqua_username');
	$password = variable_get('eaton_eloqua_password');

	// Request params
	$data = array(
		'scope' => 'full',
	);

	if (is_null($refresh_token)) {
		// If no refresh token is passed, authorize using password grant_type
		$data['grant_type'] = 'password';
		$data['username'] = $username;
		$data['password'] = $password;
	}
	else {
		// Otherwise try a refresh
		$data['grant_type'] = 'refresh_token';
		$data['refresh_token'] = $refresh_token;
	}

	// Use auth string as Basic HTTP Auth, passing credentials in 
	// request body
	$options = array(
		'headers' => array(
			'Authorization' => 'Basic ' . $auth_string,
			'Content-Type' => 'application/json'
		),
		'data' => json_encode($data),
		'method' => 'POST'
	);

	return drupal_http_request($url, $options);
}

/**
 * Get access token stored or from Eloqua API
 * @return {string} access_token
 */
function eaton_eloqua_get_access_token() {
	$access_token = variable_get('eaton_eloqua_access_token');
	$expires_on = variable_get('eaton_eloqua_access_expires');

	if (is_null($access_token)) {
		$response = eaton_eloqua_authorize();
		return eaton_eloqua_handle_access_token_response($response);
	}
	else if (time() >= $expires_on) {
		$refresh_token = variable_get('eaton_eloqua_refresh_token');
		$response = eaton_eloqua_authorize($refresh_token);
		return eaton_eloqua_handle_access_token_response($response);
	}
	
	return $access_token;
}

/**
 * Handle HTTP response from get_access_token().
 * @param  {obj}  $response [HTTP response object]
 * @return {string|bool} access_token or FALSE if fails
 */
function eaton_eloqua_handle_access_token_response($response) {
	if ($response->code == 200) {
		$data = json_decode($response->data);
		
		variable_set('eaton_eloqua_access_token', $data->access_token);
		variable_set('eaton_eloqua_access_expires', time() + $data->expires_in);

		if (isset($data->refresh_token)) {
			variable_set('eaton_eloqua_refresh_token', $data->refresh_token);
		}

		return $data->access_token;
	}

	return FALSE;
}

/*************
 * Field IDs *
 *************/

/**
 * Gather field IDs from saved variables.
 * @return {array} of field IDs
 */
function eaton_eloqua_form_field_ids() {
	$names = array(
		'firstName', 'lastName', 'company', 'title', 'jobRole1', 
		'industry1', 'emailAddress', 'city', 'country', 'stateProv', 
		'zipPostal', 'eventName', 'dateOfVisit', 'checkedIn', 'facilityLocation'
	);

	$fields = array();

	foreach($names as $name) {
		$fields[$name] = variable_get('eaton_eloqua_form_field__' . $name);
	}

	return $fields;
}

/**
 * Update field IDs from Eloqua API
 * @return {void}
 */
function eaton_eloqua_update_form_field_ids() {
	$form = eaton_eloqua_get_form();

	foreach($form->elements as $element) {
		variable_set('eaton_eloqua_form_field__' . $element->htmlName, $element->id);
	}
}

/**
 * Return currently configured form id.
 * @return {int} [form id]
 */
function eaton_eloqua_get_form_id() {
	$id = variable_get('eaton_eloqua_form_id');
	if (is_null($id) || $id == '') {
		return EATON_ELOQUA_FORM_ID;
	}
	return $id;
}

/*************************
 *	CRUD                 *
 *************************/

/**
 * General API request to Eloqua
 * @param  {string} $url    	[URL route to take]
 * @param  {string} $method 	[HTTP method]
 * @param  {array}  $data   	[request body]
 * @return {obj} [HTTP Response]
 */
function eaton_eloqua_api_request($url, $method = 'GET', $data = array()) {
	$token = eaton_eloqua_get_access_token();
	if (!$token) {
		throw new Exception("Cannot get Eloqua Access Token.", 1);
	}

	$options = array(
		'headers' => array(
			'Authorization' => 'Bearer ' . $token,
			'Content-Type' => 'application/json'
		),
		'method' => $method,
		'data' => json_encode($data)
	);

	return drupal_http_request($url, $options);
}

/**
 * Get form details from Eloqua
 * @link http://docs.oracle.com/cloud/latest/marketingcs_gs/OMCAB/index.html#Developers/RESTAPI/1.0 Endpoints/Forms/get-assets-form.htm
 *
 * @param  {int} $form_id [form id on Eloqua]
 * @return {obj} [HTTP Response]
 */
function eaton_eloqua_get_form($form_id = NULL) {
	$form_id = !is_null($form_id) ? $form_id : eaton_eloqua_get_form_id();
	
	$response = eaton_eloqua_api_request('https://secure.eloqua.com/API/REST/1.0/assets/form/' . $form_id);

	if ($response->code == 200) {
		return json_decode($response->data);
	}
	else {
		throw new Exception("Response from Eloqua invalid: " . $response->data, $response->code);
	}
}

/**
 * Get form data from Eloqua
 * @link http://docs.oracle.com/cloud/latest/marketingcs_gs/OMCAB/index.html#Developers/RESTAPI/1.0 Endpoints/Form data/get-assets-formData.htm
 *
 * @param  {int} $form_id [form id on Eloqua]
 * @return {obj} [HTTP Response]
 */
function eaton_eloqua_get_form_data($form_id = NULL) {
	$form_id = !is_null($form_id) ? $form_id : eaton_eloqua_get_form_id();

	$response = eaton_eloqua_api_request('https://secure.eloqua.com/API/REST/1.0/data/form/' . $form_id);

	if ($response->code == 200) {
		return json_decode($response->data);
	}
	else {
		throw new Exception("Response from Eloqua invalid: " . $response->data, $response->code);
	}
}

/**
 * Create form data on Eloqua
 * @link http://docs.oracle.com/cloud/latest/marketingcs_gs/OMCAB/index.html#Developers/RESTAPI/1.0 Endpoints/Form data/post-assets-form.htm
 *
 * @param  {int} $form_id [form id on Eloqua]
 * @param  {obj} $attendee [attendee object]
 * @param  {bool} $registration_only [ignore attended field]
 * @return {obj} [HTTP Response]
 */
function eaton_eloqua_create_form_data($form_id = NULL, $attendee, $event_id, $registration_only = FALSE) {
	$form_id = !is_null($form_id) ? $form_id : eaton_eloqua_get_form_id();

	// Get array map of eloqua field names -> field ids
	$fields = eaton_eloqua_form_field_ids();

	// Map attendee properties to Eloqua field names
	$substitutions = array(
		'email' => 'emailAddress',
		'state' => 'stateProv',
		'zip' => 'zipPostal',
		'visitor_type' => 'jobRole1',
		'market_focus' => 'industry1'
	);

	// Begin HTTP request body
	$data = array(
		'id' => $form_id,
		'submittedAt' => time(),
		'fieldValues' => array()
	);

	// Build out fieldValues array
	foreach($attendee as $key => $value) {
		switch ($key) {
			case 'attendee_id':
			case 'picture':
			case 'created':
			case 'events':
				// skip
				break;
			case 'check_status':
				if (!$registration_only) {
					$most_recent = array('status' => '', 'timestamp' => 0);
					foreach($value as $check_status) {
						if ($check_status->event_id == $event_id && $check_status->timestamp > $most_recent['timestamp']) {
							//$status = $check_status->type == 'in' ? 1 : 0;
							$most_recent = array('status' => 1, 'timestamp' => $check_status->timestamp);
						}
					}
					if ($most_recent['timestamp'] > 0) {
						$data['fieldValues'][] = array(
							'type' => 'FieldValue',
							'id' => $fields['dateOfVisit'],
							'value' => $most_recent['timestamp']
						);
						$data['fieldValues'][] = array(
							'type' => 'FieldValue',
							'id' => $fields['checkedIn'],
							'value' => $most_recent['status']
						);
					}
				}
				break;
			default:
				// Try subsitutions first
				if (isset($substitutions[$key])) {
						$data['fieldValues'][] = array(
							'type' => 'FieldValue',
							'id' => $fields[$substitutions[$key]],
							'value' => $value
						);
				}
				else {
					// Otherwise, convert key to camel case and try to find
					// in Eloqua form field -> id map
					$camelCase = _camelize($key);
					if (isset($fields[$camelCase])) {
						$data['fieldValues'][] = array(
							'type' => 'FieldValue',
							'id' => $fields[$camelCase],
							'value' => $value
						);
					}
				}
		}
	}

	// Get the event (node)
	$event = eventresource_retrieve_one($event_id, '');
	$wrapper = entity_metadata_wrapper('node', $event);

	// Append the event title and location to field values
	$data['fieldValues'][] = array(
		'type' => 'FieldValue',
		'id' => $fields['eventName'],
		'value' => $event->title
	);
	$data['fieldValues'][] = array(
		'type' => 'FieldValue',
		'id' => $fields['facilityLocation'],
		'value' => $wrapper->field_event_location->value()
	);

	// Return response
	return eaton_eloqua_api_request('https://secure.eloqua.com/API/REST/1.0/data/form/' . $form_id, 'POST', $data);
}

/**
 * Helper string functions
 */
function _camelize($str) {
	$parts = explode('_', $str);
	return array_shift($parts) . implode('',explode(' ',ucwords(implode(' ', $parts))));
}

/**
 * @link http://stackoverflow.com/questions/1993721/how-to-convert-camelcase-to-camel-case
 */
function _decamelize($word) {
      return $word = preg_replace_callback(
        "/(^|[a-z])([A-Z])/",
        function($m) { return strtolower(strlen($m[1]) ? "$m[1]_$m[2]" : "$m[2]"); },
        $word
    );

}

/**
 * Cron worker queue callback
 * ---------------------------
 * Posts data to Eloqua form
 * -----------------------------
 * @see eaton_backbone_cron_queue_info()
 * @see eaton_backbone_cron()
 * ----------------------------------
 * 
 * @param  {array} $record [stdClass representing row in eaton_eloqua_MYSQL table]
 * @return {void}
 */
function eaton_eloqua_update_remote($record) {
	$attendee = _attendeeresource_get_attendee($record->attendee_id);
	$registration_only = ($record->action == 'registered');
	$response = eaton_eloqua_create_form_data(NULL, $attendee, $record->event_id, $registration_only);
	if ($response->code == 201) {
	  db_update('eaton_backbone_eloqua')
	    ->fields(array(
	        'completed' => 1,
	        'updated_at' => time()
	      ))
	    ->condition('id', $record->id)
	    ->execute();
	}
}


/**
 * Administration Configuration Form
 * 
 * Allow the user to set client id, client secret, username, and password
 * for conecting to Eloqua.
 * 
 * @return {array} [drupal form]
 */
function eaton_eloqua_config_form() {
	$form['eaton_eloqua_client_id'] = array(
		'#type' => 'textfield',
		'#title' => 'Client ID',
		'#required' => TRUE,
		'#description' => t('Client ID provided by Eloqua'),
		'#default_value' => variable_get('eaton_eloqua_client_id', '')
	);
	$form['eaton_eloqua_client_secret'] = array(
		'#type' => 'textfield',
		'#title' => 'Client Secret',
		'#required' => TRUE,
		'#description' => t('Client Secret provided by Eloqua'),
		'#default_value' => variable_get('eaton_eloqua_client_secret', '')
	);
	$form['eaton_eloqua_username'] = array(
		'#type' => 'textfield',
		'#title' => 'Username',
		'#required' => TRUE,
		'#description' => t('Username to authorize with Eloqua'),
		'#default_value' => variable_get('eaton_eloqua_username', '')
	);
	$form['eaton_eloqua_password'] = array(
		'#type' => 'textfield',
		'#title' => 'Password',
		'#attributes' => array('style' => '-webkit-text-security: disc; text-security: disc; -moz-text-security: disk; -ms-text-security: disk;'),
		'#required' => TRUE,
		'#description' => t('Password to authorize with Eloqua'),
		'#default_value' => variable_get('eaton_eloqua_password', '')
	);
	$form['eaton_eloqua_form_id'] = array(
		'#type' => 'textfield',
		'#title' => 'Form ID',
		'#required' => FALSE,
		'#description' => t('Form ID to target on Eloqua.  Leave blank to default to form ' . EATON_ELOQUA_FORM_ID),
		'#default_value' => variable_get('eaton_eloqua_form_id', '')
	);

	$form['test_configuration'] = array(
		'#type' => 'submit',
		'#submit' => array('eaton_eloqua_config_form_test'),
		'#value' => t('Test Configuration')
	);
	return system_settings_form($form);
}

function eaton_eloqua_config_form_test($form, &$form_state) {
	$access_token = eaton_eloqua_get_access_token();
	if ($access_token) {
		try {
			$form = eaton_eloqua_get_form();
			drupal_set_message('Connection to Eloqua is working.');
		}
		catch (Exception $e) {
			drupal_set_message('Connection to Eloqua is not working!  Exception: ' . $e->message, 'error');
		}
	}
	else {
		drupal_set_message('Connection to Eloqua is not working!  Cannot get access token.', 'error');
	}
}