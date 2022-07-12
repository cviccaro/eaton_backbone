<?php
/**
 * Underscore.JS templates
 * eaton_backbone_event_request_coordinator_node_template: Template for instance of event request node (for coordinators)
 * eaton_backbone_event_request_initiator_node_template: Template for instance of event request node (for initiators)
 * eaton_backbone_event_attendees_attendee_template: Template for instance of Event Attendee (shown on event's individual page)
 *
 * Other themes

 * eaton_backbone_event_details: Theme returning Event Request details
 * eaton_backbone_attendee_details: Theme returning Attendee details
 * eaton_backbone_approved_email: Theme returning HTML email for event approval
 * eaton_backbone_registration_email: Theme returning HTML email for attendee registration
 * eaton_backbone_public_registration_thanks: Theme for public registration thank-you page
 * eaton_backbone_event_attendees: Theme returning event attendees for an event
 * eaton_backbone_my_registrations: Theme returning an attendees' registrations
 *
 */



/**
  * UNDERSCORE/BACKBONE TEMPLATES
 */
function theme_eaton_backbone_event_request_coordinator_node_template()
{
    $output = '<td class="views-field views-field-created" id="date-<%= nid %>"><%= created_local %></td>
              <td class="views-field views-field-title"><%= title %> (<%= nid %>)</td>
             <td class="views-field views-field-event-initiator" id="uid-<%= nid %>"><%= typeof(field_event_initiator) != "undefined" && typeof(field_event_initiator.und) != "undefined" ? field_event_initiator.und[0].value : Drupal.settings.event_requests.users[uid].field_first_name_value + " " + Drupal.settings.event_requests.users[uid].field_last_name_value %></td>
              <td class="views-field views-field-possible-date-1"><% if (typeof(field_possible_dates) != "undefined" && typeof(field_possible_dates.und) != "undefined") { %><%= dateRange(field_possible_dates.und[0]) %><% } %></td>          
              <td class="views-field views-field-updated"><time class="timeago" title="<%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %>" datetime="<%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %>"><%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %></time></td>
              <td class="views-field views-field-operations">
              <a href="/event/<%= nid %>/edit" class="edit-button" title="Edit Event Request">Edit</a>
              <a href="/event/<%= nid %>/delete" class="delete-button" title="Delete Event Request">Delete</a><br />
                <% if (typeof(field_request_status) != "undefined" && typeof(field_request_status.und) != "undefined") { %> 
                <% if(field_request_status.und[0].value == "review") { %>
                  <a class="approve-button" href="#" title="Approve Event Request">Approve</a> 
                  <a class="deny-button" href="#" title="Deny Event Request">Deny</a><br />
              <% } else if (field_request_status.und[0].value == "denied") { %>
              <a class="approve-button" href="#" title="Approve Event Request">Approve</a>
              <% } else if (field_request_status.und[0].value == "approved") { %>
              <a class="register-button" href="/event/<%= nid %>" title="View registrations and preregistrations, and view attendees\' badges">Preregister</a>
              <% if (typeof(field_event_visibility) != "undefined" && typeof(field_event_visibility.und) != "undefined" && field_event_visibility.und[0].value == "private") { %>
                <a class="register-button" href="register/<%= nid %>" title="Private Registration Link">Private Registration Link</a>
              <% }
              else { %>
                  <a class="register-button" href="register/<%= nid %>" title="Public Registration Link">Public Registration Link</a>
              <% } %>
              <% } } %>
              </td>';
    return $output;
}
function theme_eaton_backbone_event_request_initiator_node_template()
{
      return '<td class="views-field views-field-created" id="date-<%= nid %>"><%= created_local %></td>
              <td class="views-field views-field-title"><%= title %></td>
              <td class="views-field views-field-event-location" id="location-<%= nid %>"><%= valueForKey("field_event_location", field_event_location.und[0].value) %></td>
              <td class="views-field views-field-event-visibility" id="visibility-<%= nid %>">
                <% if (typeof(field_event_visibility) != "undefined" && typeof(field_event_visibility.und) != "undefined") { %> <%= valueForKey("field_event_visibility", field_event_visibility.und[0].value) %> <% } %>
              </td>              
              <td class="views-field views-field-possible-date-1"><% if (typeof(field_possible_dates) != "undefined" && typeof(field_possible_dates.und) != "undefined") { %><%= dateRange(field_possible_dates.und[0]) %><% } %></td>
              <td class="views-field views-field-possible-date-2"><% if (typeof(field_possible_dates) != "undefined" && typeof(field_possible_dates.und) != "undefined" && typeof(field_possible_dates.und[1]) != "undefined") { %><%= dateRange(field_possible_dates.und[1]) %><% } %></td>
              <td class="views-field views-field-possible-date-3"><% if (typeof(field_possible_dates) != "undefined" && typeof(field_possible_dates.und) != "undefined" && typeof(field_possible_dates.und[2]) != "undefined") { %><%= dateRange(field_possible_dates.und[2]) %><% } %></td>
              <td class="views-field views-field-denied-suggested-date"><% if (typeof(field_denied_suggested_date) != "undefined" && typeof(field_denied_suggested_date.und) != "undefined" && typeof(field_denied_suggested_date.und[0]) != "undefined") { %><%= dateRange(field_denied_suggested_date.und[0]) %><% } %></td>              
<td class="views-field views-field-request-status"><% if (typeof(field_request_status) != "undefined" && typeof(field_request_status.und) != "undefined") { %><%= valueForKey("field_request_status", field_request_status.und[0].value) %><% } %></td>              
              <td class="views-field views-field-updated"><time class="timeago" title="<%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %>" datetime="<%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %>"><%= new Date(changed * 1000).toISOString().replace(\'"\',\'\') %></time></td>
              <td class="views-field views-field-operations">
                <% if (typeof(field_request_status) != "undefined" && field_request_status.und[0].value == "approved") { %>
                  <a class="register-button" href="/event/<%= nid %>" title="Preregister your attendees and view their badges">Preregister</a>
                  <% if (typeof(field_event_visibility) != "undefined" && typeof(field_event_visibility.und) != "undefined" && field_event_visibility.und[0].value == "private") { %>
                    <a class="register-button" href="register/<%= nid %>" title="Private Registration Link">Private Customer Registration Link</a>
                  <% }
                     else { %>
                      <a class="register-button" href="register/<%= nid %>" title="Public Registration Link">Public Registration Link</a>
                  <% } %>
                <% } %>
              </td>
            ';
}

function theme_eaton_backbone_event_attendees_attendee_template()
{
    return '<td class="views-field field-attendee-id" style="display:none"><%= attendee_id %></td>
          <td class="views-field field-name"><%= first_name %> <%= last_name %></td>
          <td class="views-field field-email"><%= email %></td>
          <td class="views-field field-date-registered"><%= datestampToDate(timestamp) %></td>
          <td class="views-field field-checked-in"><%= evaluateCheckInOut(check_status) %></td>
          <td class="views-field field-operations">
          <a href="#" class="edit-button" title="Edit Attendee">Edit</a> - 
          <a href="#" class="remove-button" title="Remove Attendee">Remove</a> - 
          <a href="#" class="badge-link badge-button" title="View Badge - Front">Badge Front</a> - 
          <a href="#" class="check-inout-button check-in" title="Check In">Check In</a> -
          <a href="#" class="check-inout-button check-out" title="Check Out">Check Out</a>
          </td>
          ';
}
 
/**
 * EMAIL TEMPLATES
 */

function theme_eaton_backbone_request_email($variables)
{
    $node = $variables['node'];
    $theme_key = 'eaton';
    $logo = theme_get_setting('logo', $theme_key);
    $output = '<div class="email-wrapper">';
    $receipt_id = _eaton_backbone_encrypt_receipt_id($node->nid);
    $output .= '<div class="email-link">' . l('Not appearing correctly? View this email online', 'receipt/' . $receipt_id, array('absolute' => true)) . '</div>';
    $output .= '<div class="eaton-event-request-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= theme('eaton_backbone_event_request_receipt', array('node' => $node));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
  
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<link rel="stylesheet" href="' . url($css_path, array('absolute' => true)) . '" />';
    }
    return $output;
}
function theme_eaton_backbone_approved_email($variables)
{
  //global $theme_key;
    $theme_key = 'eaton';
    $node = $variables['node'];
    $email_id = _eaton_backbone_encrypt_email_id($node->nid);
    $logo = theme_get_setting('logo', $theme_key);
    $output = '<div class="email-wrapper">';
    $output .= '<div class="email-link">' . l('Not appearing correctly? View this email online', 'email/' . $email_id, array('absolute' => true)) . '</div>';
    $output .= '<div class="eaton-event-approved-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= l('<h1 id="title">' . $node->title . '</h1>', 'register/' . $node->nid, array('html' => true, 'absolute' => true));
  

  //$output .= '<p>Praesent ac arcu eros. Mauris malesuada, mi in feugiat vehicula, nibh dui pulvinar nibh, et imperdiet dolor nisl in mauris. Aliquam feugiat pretium dui sed eleifend. Quisque turpis turpis, dictum sed ipsum a, tristique consectetur sapien. Suspendisse id elit et dolor tempus semper sit amet eu urna. Praesent ac ornare odio, ac tempor orci. Nulla egestas iaculis eleifend.</p>';
    $output .= _eaton_email_template_replace_variables(variable_get('eaton_email_template_approved', ''), $node);

    $to_render = array('field_approved_date','field_event_location', 'field_vista_number');

    foreach ($to_render as $field_name) {
        if ($field_name == 'field_approved_date') {
            $display = array(
              'label' => 'above',
              'type' => 'date_default',
              'settings' => array(
            'format_type' => 'medium',
              ),
            );
            $field_view = field_view_field('node', $node, $field_name, $display);
        } else {
            $field_view = field_view_field('node', $node, $field_name);
        }
        $field_view['#label_display'] = 'hidden';
        $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
        $field_view['#suffix'] = '</div>';
        switch ($field_name) {
            case 'field_approved_date':
                $field_view['#title'] = 'Event Date';
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                break;
            case 'field_event_location':
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                $field_view[0]['#markup'] .= ' Office';
                break;
        }
    
    
        if (!empty($field_view['#items'])) {
            $output .= render($field_view);
        }
    }
    $is_public = _eaton_backbone_event_is_public($node);
    if ($is_public) {
        $output .= '<div class="event-register-link">' . l('Public Registration Link', 'register/' . $node->nid, array('absolute' => true)) . '</div>';
    } else {
        $output .= '<div class="event-register-link">' . l('Private Registration (Pre-registration only)', 'register/' . $node->nid, array('absolute' => true)) . '</div>';
    }
    $output .= '<div class="email-actions">';
    $output .= l('Add to Calendar', 'ics/' . $node->nid, array('absolute' => true, 'attributes' => array('class' => array('big-button'))));
    $output .= '&nbsp;&nbsp;&nbsp;&nbsp;';
    $output .= l('Register Your Attendees', 'event/' . $node->nid, array('absolute' => true,'attributes' => array('class' => array('big-button'))));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
  
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<link rel="stylesheet" href="' . url($css_path, array('absolute' => true)) . '" />';
    }
  
    $output .= "</div>";
    return $output;
}


function theme_eaton_backbone_denied_email($variables)
{
  //global $theme_key;
    $theme_key = 'eaton';
    $node = $variables['node'];
    $email_id = _eaton_backbone_encrypt_email_id($node->nid);
    $location = eaton_backbone_eaton_location_by_event($node);
    $logo = theme_get_setting('logo', $theme_key);
    $output = '';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<style type="text/css">' . file_get_contents(DRUPAL_ROOT . '/' . $css_path) . '</style>';
    }
    $output .= '<div class="email-wrapper">';
    $output .= '<div class="email-link">' . l('Not appearing correctly? View this email online', 'email/' . $email_id, array('absolute' => true)) . '</div>';
    $output .= '<div class="eaton-event-denied-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= '<h1 id="title">Your Event Request has been Denied</h1>';
  
  //$output .= '<p>Praesent ac arcu eros. Mauris malesuada, mi in feugiat vehicula, nibh dui pulvinar nibh, et imperdiet dolor nisl in mauris. Aliquam feugiat pretium dui sed eleifend. Quisque turpis turpis, dictum sed ipsum a, tristique consectetur sapien. Suspendisse id elit et dolor tempus semper sit amet eu urna. Praesent ac ornare odio, ac tempor orci. Nulla egestas iaculis eleifend.</p>';
    $output .= _eaton_email_template_replace_variables(variable_get('eaton_email_template_denied', ''), $node);

    $to_render = array('field_denied_suggested_date','field_denied_message');
    foreach ($to_render as $field_name) {
        $display = array();
        if ($field_name == 'field_denied_suggested_date') {
            $display = array(
            'label' => 'above',
            'type' => 'date_default',
            'settings' => array(
              'format_type' => 'medium',
            ),
            );
        }
        $field_view = field_view_field('node', $node, $field_name, $display);
        $field_view['#label_display'] = 'hidden';
        $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
        $field_view['#suffix'] = '</div>';
        switch ($field_name) {
            case 'field_denied_suggested_date':
                $field_view['#label_display'] = 'above';
                $field_view['#title'] = 'Suggested Date and Time';
                $field_view['#view_mode'] = 'default';
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                break;
            case 'field_event_location':
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                $field_view[0]['#markup'] .= ' Office';
                break;
        }
    
    
        if (!empty($field_view['#items'])) {
            switch ($field_name) {
                case 'field_denied_message':
                    $pre_output = render($field_view);
                    $output .= html_entity_decode($pre_output, ENT_QUOTES);
                    break;
                default:
                    $output .= render($field_view);
            }
        }
    }

    $mailto = '';
    if (!empty($location->location_contact)) {
        $mailto .= '"' . $location->location_contact . '" ';
        $mailto .= "<$location->location_email>";
        $mailto = rawurlencode($mailto);
    } else {
        $mailto .= $location->location_email;
    }
    $mailto .= '?Subject=Regarding Denial of Event ' . $node->title;
    $mailto = "mailto:$mailto";

    $output .= '<div class="email-actions">';
    $output .= l('Accept Suggested Date', 'event/' . $node->nid . '/accept_suggested_date', array('absolute' => true, 'attributes' => array('class' => array('big-button'))));
    $output .= '&nbsp;&nbsp;';
    $output .= l('Contact Coordinator', $mailto, array('absolute' => true, 'attributes' => array('class' => array('big-button'))));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';

  
    $output .= "</div>";
    return $output;
}

function theme_eaton_backbone_event_email($variables)
{
    $node = $variables['node'];
    $theme_key = 'eaton';
    $logo = theme_get_setting('logo', $theme_key);
    $output = '<div class="email-wrapper">';
    $receipt_id = _eaton_backbone_encrypt_receipt_id($node->nid);
    $output .= '<div class="email-link">' . l('Not appearing correctly? View this email online', 'receipt/' . $receipt_id, array('absolute' => true)) . '</div>';
    $output .= '<div class="eaton-event-request-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= theme('eaton_backbone_event_request_receipt', array('node' => $node));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
  
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<link rel="stylesheet" href="' . url($css_path, array('absolute' => true)) . '" />';
    }
    return $output;
}

function theme_eaton_backbone_registration_email($variables)
{
  //global $theme_key;
  
    $theme_key = 'eaton';
    $node = $variables['node'];
    $attendee = $variables['attendee'];
    $receipt_id = $variables['receipt_id'];
    $message = $variables['message'];
    $logo = theme_get_setting('logo', $theme_key);
  
    $output = '<div class="email-wrapper">';
    $output .= '<div class="email-link">' . l('Not appearing correctly? View this email online', 'email/' . $receipt_id, array('absolute' => true)) . '</div>';
    $output .= '<div class="eaton-event-public-registration-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= '<h1 id="title">' . $node->title . '</h1>';
    $output .= '<p>' . t($message, array('@first_name' => $attendee->first_name, '@last_name' => $attendee->last_name)) . '</p>';
  //$output .= '<p>Praesent ac arcu eros. Mauris malesuada, mi in feugiat vehicula, nibh dui pulvinar nibh, et imperdiet dolor nisl in mauris. Aliquam feugiat pretium dui sed eleifend. Quisque turpis turpis, dictum sed ipsum a, tristique consectetur sapien. Suspendisse id elit et dolor tempus semper sit amet eu urna. Praesent ac ornare odio, ac tempor orci. Nulla egestas iaculis eleifend.</p>';
    $output .= _eaton_email_template_replace_variables(variable_get('eaton_email_template_attendee_registration', ''), $node);
    $output .= '<p class="request-status-ics">'. l('Add to Calendar', 'ics/' . $node->nid, array('absolute' => true, 'attributes' => array('class' => array('big-button')))) . '</p>';
    $output .= '<p>' . theme_eaton_backbone_attendee_details(array('attendee' => $attendee)) . '</p>';
  
    $to_render = array('field_approved_date','field_event_location');
    foreach ($to_render as $field_name) {
        $field_view = field_view_field('node', $node, $field_name);
        $field_view['#label_display'] = 'hidden';
        $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
        $field_view['#suffix'] = '</div>';
        switch ($field_name) {
            case 'field_approved_date':
                $field_view['#title'] = 'Event Date';
                break;
            case 'field_event_location':
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                $field_view[0]['#markup'] .= ' Office';
                break;
        }
    
    
        $output .= render($field_view);
    }

    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
  
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<link rel="stylesheet" href="' . url($css_path, array('absolute' => true)) . '" />';
    }
    $output .= "</div>";
    return $output;
}
function theme_eaton_backbone_public_registration_thanks($variables)
{
    $event_id = $variables['event_id'];
    $attendee_id = $variables['attendee_id'];
    $attendee = _attendeeresource_get_attendee($attendee_id);
    $node = node_load($event_id);
  
    $output = '<div class="email-wrapper">';
    $output = '<h2>Thank you for registering for the <em>' . $node->title . '</em> event.  Below is a copy of the event details for your reference.</h2>';
    $output .= theme('eaton_backbone_event_details', array('node' => array('node' => $node), 'add_ical' => TRUE));
    $output .= theme('eaton_backbone_attendee_details', array('attendee' => $attendee));
    $output .= "</div>";
  
    return $output;
}




/**
 * Content templates
 */

function theme_eaton_backbone_event_details($variables)
{
    if (!isset($variables['add_ical'])) {
        $variables['add_ical'] = false;
    }
    $render_array = $variables['node'];
    $node = $render_array['node'];
  
    $output = '';
  
    $to_render = [
      'field_event_initiator',
      'field_event_initiator_email',
      'field_psec_support',
      'field_psec_support_provided_by',
      'field_interests_topics'
    ];
    foreach ($to_render as $field_name) {
        if ($field_name == 'field_approved_date') {
            $display = array(
              'label' => 'above',
              'type' => 'date_default',
              'settings' => array(
            'format_type' => 'medium',
              ),
            );
            $field_view = field_view_field('node', $node, $field_name, $display);
        } else {
            $field_view = field_view_field('node', $node, $field_name);
        }
    
        $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
        switch ($field_name) {
            case 'field_approved_date':
                $field_view['#title'] = 'Event Date';
                break;
            case 'field_interests_topics':
                  $field_view['#title'] = 'Event Description';
                  break;
        }
        $field_view['#suffix'] = '</div>';
    
        $output .= render($field_view);
    }
    if ($variables['add_ical'] == true) {
        $output .= '<p class="request-status-ics" style="float:left;clear:both;width:100%;">'. l('Add to Calendar', 'ics/' . $node->nid, array('absolute' => true, 'attributes' => array('target' => '_blank', 'class' => array('big-button')))) . '</p>';
    }
    return '<div class="eaton-event-details">' . $output . '</div>';
}

function theme_eaton_backbone_event_details_invite($variables)
{
    if (!isset($variables['add_ical'])) {
        $variables['add_ical'] = false;
    }
    $render_array = $variables['node'];
    $node = $render_array['node'];
    $wrapper = entity_metadata_wrapper('node', $node);
  
    $output = '<h1 class="page-register-title blue">You\'re invited</h1>';

    $field = field_view_field('node', $node, 'field_interests_topics', ['label' => 'hidden', 'type' => 'text_default']);
    $output .= render($field);

    $location_code = $wrapper->field_event_location->value();
    $location = eaton_backbone_eaton_location_by_code($location_code);

    $output .= '<div class="field field-name-event-location">';
    $output .= '<h3 class="field-label">Where:</h3>';
    $output .= '<div class="field-content">' . $location->location_name . '<br />' . eaton_backbone_eaton_location_address($location_code) . '</div>';
    $output .= '</div>';

    $date = $wrapper->field_possible_dates[0]->value();
    $start_date = format_date(strtotime($date['value']), 'custom', 'F j g:iA T');
    $end_date = format_date(strtotime($date['value2']), 'custom', 'F j, Y g:iA T');

    $output .= '<div class="field field-name-event-date">';
    $output .= '<h3 class="field-label">When:</h3>';
    $output .= '<div class="field-content">' . t('@start - @end', ['@start' => $start_date, '@end' => $end_date]) . '</div>';
    $output .= '</div>';


  
    // $to_render = [
    //   'field_event_initiator',
    //   'field_event_initiator_email',
    //   'field_psec_support',
    //   'field_psec_support_provided_by',
    //   'field_interests_topics'
    // ];
    // foreach ($to_render as $field_name) {
    //     if ($field_name == 'field_approved_date') {
    //         $display = array(
    //           'label' => 'above',
    //           'type' => 'date_default',
    //           'settings' => array(
    //         'format_type' => 'medium',
    //           ),
    //         );
    //         $field_view = field_view_field('node', $node, $field_name, $display);
    //     } else {
    //         $field_view = field_view_field('node', $node, $field_name);
    //     }
    
    //     $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
    //     switch ($field_name) {
    //         case 'field_approved_date':
    //             $field_view['#title'] = 'Event Date';
    //             break;
    //         case 'field_interests_topics':
    //               $field_view['#title'] = 'Event Description';
    //               break;
    //     }
    //     $field_view['#suffix'] = '</div>';
    
    //     $output .= render($field_view);
    // }
    if ($variables['add_ical'] == true) {
        $output .= '<p class="request-status-ics" style="float:left;clear:both;width:100%;">'. l('Add to Calendar', 'ics/' . $node->nid, array('absolute' => true, 'attributes' => array('target' => '_blank', 'class' => array('big-button')))) . '</p>';
    }
    return '<div class="eaton-event-details">' . $output . '</div>';
}


function theme_eaton_backbone_attendee_details($variables)
{
  
    if (isset($variables['attendee']) && !is_null($variables['attendee']) && is_object($variables['attendee'])) {
        $attendee = $variables['attendee'];
    } else {
        $attendee = _attendeeresource_get_attendee($variables['attendee_id']);
    }
    $title = isset($variables['title']) ? $variables['title'] : '';
  
    $output = '';
    if (!empty($title)) {
        $output .= '<h2>' . $title . '</h2>';
    }
    $output .= '<p>' . $attendee->first_name . ' ' . $attendee->last_name . '<br />';
    $output .= $attendee->email . '<br />';
  
    $address = $attendee->city;
    if (!empty($attendee->state)) {
        $address .= ', ' . $attendee->state;
    }
    $address .= ', ' . $attendee->country;
    if (!empty($attendee->zip)) {
        $address .= ', ' . $attendee->zip;
    }
    $output .= $address . '</p>';
    if (!empty($variables['extra'])) {
        $output .= $variables['extra'];
    }
    $classes = array('attendee-details');
    if (!empty($variables['class'])) {
        $classes = array_merge($classes, $variables['class']);
    }
    return '<div id="attendee-details-' . $attendee->attendee_id . '" class="' . implode(' ', $classes) . '">' . $output . '</div>';
}

/**
 * Returns a Drupal table displaying all attendees for an event
 */

function theme_eaton_backbone_event_attendees($variables)
{
    global $user;
    $is_coordinator = _eaton_backbone_is_coordinator($user);
  
// Add badge links JS
    drupal_add_js(drupal_get_path('module', 'eaton_backbone') . '/js/badge-links.js');
  
    $node = $variables['node'];
  
    $output = '<div class="eaton-backbone-event-attendees" id="eaton-backbone-event-attendees-' . $node->nid . '">';

    $approved_date = field_view_field('node', $node, 'field_approved_date', array('label' => 'hidden', 'settings' => array('format_type' => 'medium')));
    $output .= '<h2 class="subtitle">' . $node->title . '</h2>' . render($approved_date);
  
    $header = array(
    'name' => t('Full Name'),
    'email' => t('Email'),
    'date_registered' => t('Date Registered'),
    'checked_in' => t('Checked-In'),
    'badge' => t('Badge'),
    );
  
  // Add admin-side stuff
    if ($is_coordinator) {
        $header['operations'] = t('Operations');
        drupal_add_js(drupal_get_path('module', 'eaton_backbone') . '/js/today-page.js');
    }
    $attendees = _attendeeresource_event_attendees($node->nid);
    $output .= '<p>Total Registered Attendees: ' . count($attendees) . '</p>';
    $output .= '<div class="app-actions"><a href="/badges/' . $node->nid . '" target="_blank" id="view_all_badges" class="button" style="display: inline-block;">View All Badges</a></div>';
    $x = 0;
    $rows = array();
    foreach ($attendees as $attendee_id => $attendee) {
        $row = array(
        'name' => array('data' => $attendee->first_name . ' ' . $attendee->last_name, 'class' => array('field-name')),
        'email' => array('data' => $attendee->email, 'class' => array('field-email')),
        'date_registered' => array('data' => format_date($attendee->timestamp, 'medium'), 'class' => array('field-date-registered')),
        'checked_in' => array('data' => eaton_backbone_evaluate_checked_in_out($attendee, $node->nid),'class' => array('field-checked-in')),
        'badge' => array(
        'data' => l('View Badge - Front', 'badge/' . $node->nid . '/' . $attendee->attendee_id . '/front', array('attributes' => array('class' => 'badge-link badge-button-front', 'title' => 'View Badge Front'))) .
                  '<br />' . l('View Badge - Rear', 'badge/' . $node->nid . '/' . $attendee->attendee_id . '/back', array('attributes' => array('class' => 'badge-link badge-button-back', 'title' => 'View Badge Back'))),
        'class' => array('field-badge-link')),
        );
        if ($is_coordinator) {
            $row['operations'] = array('data' => '', 'class' => array('field-operations'));
            $row['operations']['data'] = '<a href="#" class="email-button" title="Send Email">Send Email</a><br />
      <a href="#" class="check-inout-button check-in" title="Check In">Check In</a> - 
      <a href="#" class="check-inout-button check-out" title="Check Out">Check Out</a>';
        }
        $rows[$attendee_id] = array('data' => $row, 'data-id' => $attendee_id);

        $x++;
    }
    $table_vars = array(
    'header' => $header,
    'rows' => $rows,
    'sticky' => false,
    'caption' => '',
    'colgroups' => array(),
    'empty' => 'No attendees have been registered for this event',
    'attributes' => array('class' => array('eaton-app-table'), 'id' => 'eaton-event-attendees-table-' . $node->nid, 'data-id' => $node->nid),
    );
    $output .= theme_table($table_vars);
    $output .= '</div>';
    return $output;
}

/**
 * Display a table of a user's registrations (as an attendee)
 */
function theme_eaton_backbone_my_registrations($variables)
{
    $attendee = $variables['attendee'];
  // Define header
    $header = array(
    'event_name' => array('field' => 'n.title', 'data' => t('Event Name'), 'type' => 'property', 'specifier' => 'title'),
    'event_date' => array('data' => t('Event Date'), 'type' => 'field', 'specifier' => array('field' => 'field_approved_date', 'column' => 'value')),
    'timestamp' => 'Date Registered'
    );
  // Build rows
    $rows = array();
    if (isset($attendee->events) && !empty($attendee->events)) {
        // Gather list of node IDs
        $nids = array();
        foreach ($attendee->events as $event_info) {
            $nids[] = $event_info->event_id;
        }
        // Fetch nodes using IDs
        $query = new EntityFieldQuery();
        $query
        ->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'event_request')
        ->propertyCondition('status', 1)
        ->entityCondition('entity_id', $nids, 'IN')
        ->tableSort($header);
        $result = $query->execute();
        if ($result) {
            // Loop through nodes
            foreach ($result['node'] as $node) {
                // Get a receipt ID to link to a registration email
                $receipt_id = _eaton_backbone_encrypt_registration_id($node->nid, $attendee->attendee_id, 'post');
                // Load the node
                $event = _eaton_backbone_event($node->nid);
                $display = array(
                'label' => 'hidden',
                'type' => 'date_default',
                'settings' => array(
                  'format_type' => 'medium',
                ),
                );
                $approved_date = field_view_field('node', $event, 'field_approved_date', $display);
                //dpm($approved_date);
                $date_registered = format_date($attendee->events[$event->nid]->timestamp, 'medium');
                // Add to rows
                $rows[$event->nid] = array(
                'event_name' => l($event->title, 'email/' . $receipt_id, array('attributes' => array('target' => '_blank', 'title' => 'View Registration Receipt Email'))) . ' ' . l('', 'ics/' . $node->nid, array('attributes' => array('class' => array('ics-link'), 'title' => 'Download ICS file and Add to your Calendar'))),
                'event_date' => l($approved_date[0], 'ics/' . $node->nid, array('html' => true, 'attributes' => array('title' => 'Download ICS file and Add to your Calendar'))),
                'timestamp' => l($date_registered, 'email/' . $receipt_id, array('attributes' => array('target' => '_blank', 'title' => 'View Registration Receipt Email'))),
                );
            }
        }
    }
  // Pass through theme_table
    return theme_table(array('header' => $header, 'rows' => $rows, 'empty' => t('You have not registered for any events.'), 'sticky' => false, 'caption' => '', 'colgroups' => array(), 'attributes' => array('class' => array('eaton-app-table'))));
}

function theme_eaton_backbone_suggested_date_accepted_email($variables)
{
  
    $theme_key = 'eaton';
    $node = $variables['node'];
    $email_id = _eaton_backbone_encrypt_email_id($node->nid);
    $logo = theme_get_setting('logo', $theme_key);
    $output = '';
    $css_path = 'sites/all/themes/' . $theme_key .'/stylesheets/email.css';
    if (file_exists(DRUPAL_ROOT . '/'. $css_path)) {
        $output .= '<style type="text/css">' . file_get_contents(DRUPAL_ROOT . '/' . $css_path) . '</style>';
    }
    $output .= '<div class="email-wrapper">';
  //$output .= '<div class="email-link">' . l('Not appearing correctly? View this email online','email/' . $email_id,array('absolute' => TRUE)) . '</div>';
    $output .= '<div class="eaton-event-suggested-date-accepted-email"><div class="email-header"><div class="email-header-inner"><img id="logo" src="' . $logo . '" /></div></div>';
    $output .= '<div class="email-content">';
    $output .= '<h1 id="title">Initiator has accepted suggested date for denied event request.</h1>';
  
 // $output .= '<p>Praesent ac arcu eros. Mauris malesuada, mi in feugiat vehicula, nibh dui pulvinar nibh, et imperdiet dolor nisl in mauris. Aliquam feugiat pretium dui sed eleifend. Quisque turpis turpis, dictum sed ipsum a, tristique consectetur sapien. Suspendisse id elit et dolor tempus semper sit amet eu urna. Praesent ac ornare odio, ac tempor orci. Nulla egestas iaculis eleifend.</p>';
    $output .= _eaton_email_template_replace_variables(variable_get('eaton_email_template_attendee_registration', ''), $node);
  
    $to_render = array('field_denied_suggested_date');
    foreach ($to_render as $field_name) {
        $display = array();
        if ($field_name == 'field_denied_suggested_date') {
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
        }
        $field_view = field_view_field('node', $node, $field_name, $display);
        $field_view['#label_display'] = 'hidden';
        $field_view['#prefix'] = '<div class="event-detail event-detail-' . $field_name . '">';
        $field_view['#suffix'] = '</div>';
        switch ($field_name) {
            case 'field_denied_suggested_date':
                $field_view['#label_display'] = 'above';
                $field_view['#title'] = 'Accepted Suggested Date';
                $field_view['#view_mode'] = 'default';
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                break;
            case 'field_event_location':
                //$output .= '<pre>' . print_r($field_view,TRUE) . '</pre>';
                $field_view[0]['#markup'] .= ' Office';
                break;
        }
    
    
        if (!empty($field_view['#items'])) {
            $output .= render($field_view);
        }
    }
    $output .= '<div class="email-actions">';
    $output .= l('Edit and Approve Event', 'event/' . $node->nid . '/edit', array('absolute' => true, 'attributes' => array('class' => array('big-button'))));
  //$output .= l('Request New Event','request-event',array('absolute' => TRUE, 'attributes' => array('class' => array('big-button'))));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '</div>';
    $output .= '<div class="email-footer">Copyright &copy; ' . date('Y') . ' Eaton Corporation</div>';

  
    $output .= "</div>";
    return $output;
}

function _eaton_email_template_variables()
{
    return array(
    '%my_registration_link' => l('My Registrations', 'my-registrations'),
    '%event_list_link' => l('Event List', 'events', array('absolute' => true)),
    '%approved_on_date' => 'eaton_backbone_get_event_date_approved_on',
    );
}

function _eaton_email_template_replace_variables($string, $node)
{
    $vars = _eaton_email_template_variables();
    foreach ($vars as $searchFor => $replaceWith) {
        if (function_exists($replaceWith)) {
            $replaceWith = call_user_func($replaceWith, $node);
        }
        // else {
        $string = str_replace($searchFor, $replaceWith, $string);
        // }
    }
    return $string;
}


function eaton_backbone_evaluate_checked_in_out(stdClass $attendee, $nid)
{
    $status = 'Never';
    if (isset($attendee->check_status)) {
        $newest = 0;
        $newest_status = '';
        foreach ($attendee->check_status as $tracking_id => $data) {
            if ($data->event_id == $nid) {
                if ($data->timestamp > $newest) {
                    $newest = $data->timestamp;
                    $newest_status = $data->type;
                }
            }
        }
        if ($newest > 0) {
            $status = 'Last Checked ' . ucwords($newest_status) . ' on ' . format_date($newest, 'medium');
        }
    }
    return $status;
}
