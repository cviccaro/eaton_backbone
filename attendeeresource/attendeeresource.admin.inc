<?php

function attendeeresource_admin_attendees($form, &$form_state) {
  $form['filter'] = attendeeresource_filter_form();
  $form['#submit'][] = 'attendeeresource_filter_form_submit';
  $form['admin'] = attendeeresource_settings();
  return $form;
}


/**
 * List node administration filters that can be applied.
 *
 * @return
 *   An associative array of filters.
 */
function attendeeresource_filters() {
  // Regular filters
  $filters['name'] = array('title' => 'Name');
  $filters['email'] = array('title' => 'Email');
  $filters['visitor_type'] = array('title' => 'Customer Type', 'options' => _eaton_backbone_visitor_types());
  $filters['market_focus'] = array('title' => 'Industry', 'options' => _eaton_backbone_market_focus());
  return $filters;
}

function attendeeresource_filter_form() {
  $session = isset($_SESSION['attendeeresource_filter']) ? $_SESSION['attendeeresource_filter'] : array();
  $filters = attendeeresource_filters();

  $i = 0;
  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only attendees where'),
    '#theme' => 'exposed_filters__node',
  );
  
  foreach ($session as $filter) {
    list($type, $value) = $filter;
    if ($type == 'term') {
      // Load term name from DB rather than search and parse options array.
      $value = module_invoke('taxonomy', 'term_load', $value);
      $value = $value->name;
    }
    elseif ($type == 'language') {
      $value = $value == LANGUAGE_NONE ? t('Language neutral') : module_invoke('locale', 'language_name', $value);
    }
    elseif (isset($filters[$type]['options']) && !isset($filters[$type]['value'])) {
      $value = $filters[$type]['options'][$value];
    }
    $t_args = array('%property' => $filters[$type]['title'], '%value' => $value);
    if ($i++) {
      $form['filters']['current'][] = array('#markup' => t('and where %property matches %value', $t_args));
    }
    else {
      $form['filters']['current'][] = array('#markup' => t('where %property matches %value', $t_args));
    }
    if (in_array($type, array('type', 'language'))) {
      // Remove the option if it is already being filtered on.
      unset($filters[$type]);
    }
  }

  $form['filters']['status'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('clearfix')),
    '#prefix' => ($i ? '<div class="additional-filters">' . t('and where') . '</div>' : ''),
  );
  $form['filters']['status']['filters'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('filters')),
  );
  $session_keys = array();
  if (count($session)) {
    foreach($session as $filter) {
      list($key, $value) = $filter;
      $session_keys[] = $key;
    }
  }
  foreach ($filters as $key => $filter) {
    if (!in_array($key, $session_keys)) {
      if (isset($filter['options'])) {
        $form['filters']['status']['filters'][$key] = array(
          '#type' => 'select',
          '#options' => $filter['options'],
          '#title' => $filter['title'],
          '#default_value' => '[any]',
        );  
      }
      else {
        $form['filters']['status']['filters'][$key] = array(
          '#type' => 'textfield',
          '#title' => $filter['title'],
          '#default_value' =>  isset($session[$key]) ? $session[$key] : '',
          '#attributes' => array('style' => 'margin-left: 1em'),
          '#size' => 30,
        );  
      }
    }
  }

  $form['filters']['status']['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['filters']['status']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => count($session) ? t('Refine') : t('Filter'),
  );
  if (count($session)) {
    $form['filters']['status']['actions']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['status']['actions']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  drupal_add_js('misc/form.js');

  return $form;
}

function attendeeresource_filter_form_submit($form, &$form_state) {
  $filters = attendeeresource_filters();
  switch ($form_state['values']['op']) {
    case t('Filter'):
    case t('Refine'):
      // Apply every filter that has a choice selected other than 'any'.
      foreach ($filters as $filter => $options) {
        if (isset($form_state['values'][$filter]) && $form_state['values'][$filter] != '[any]') {
            if (!isset($filter['value'])) {   
              // Flatten the options array to accommodate hierarchical/nested options.
              $flat_options = form_options_flatten($filters[$filter]['options']);
              // Only accept valid selections offered on the dropdown, block bad input.
               if (isset($flat_options[$form_state['values'][$filter]])) {
                 $_SESSION['node_overview_filter'][] = array($filter, $form_state['values'][$filter]);
               }
            }
            else {
              if (!empty($form_state['values'][$filter])) {
                $_SESSION['attendeeresource_filter'][] = array($filter, $form_state['values'][$filter]);
              }
            }
        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['attendeeresource_filter']);
      break;
    case t('Reset'):
      $_SESSION['attendeeresource_filter'] = array();
      break;
  }
}

/**
 * Applies filters for node administration filters based on session.
 *
 * @param $query
 *   A SelectQuery to which the filters should be applied.
 */
function attendeeresource_build_filter_query(SelectQueryInterface $query) {
  // Build query
  $filter_data = isset($_SESSION['attendeeresource_filter']) ? $_SESSION['attendeeresource_filter'] : array();
  foreach ($filter_data as $index => $filter) {
    list($key, $value) = $filter;
    switch ($key) {
      case 'name':
        $or = db_or();
        $or->condition('first_name', '%' . strtolower($value) .'%', 'LIKE');
        $or->condition('last_name', '%' . strtolower($value) .'%', 'LIKE');
        $query->condition($or);
        break;
      case 'email':
        $query->condition($key, '%' . strtolower($value) .'%', 'LIKE');
        break;
      case 'visitor_type':
      case 'market_focus':
        $query->condition($key, $value);
        break;
      case 'status':
        // Note: no exploitable hole as $key/$value have already been checked when submitted
        list($key, $value) = explode('-', $value, 2);
      case 'type':
      case 'language':
        $query->condition('n.' . $key, $value);
        break;
    }
  }
}
/**
 * Settings form.
 * 
 * @param type $form
 * @param type $form_state
 * @return type
 */
function attendeeresource_settings() {
  $destination = drupal_get_destination();

  // Attach Shadowbox for picture previews
  drupal_add_library('shadowbox','shadowbox');

  // Attach timeago for more semantic "Created" date
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'eaton_backbone') . '/js/jquery.timeago.js',
    'jQuery(document).ready(function() { jQuery(".timeago").timeago(); });' => array('type' => 'inline')
  );

  $header = array(
    'id' => array('data' => t('ID'), 'field' => 'a.attendee_id'),
    'first_name' => array('data' => t('First Name'), 'field' => 'a.first_name'),
    'last_name' => array('data' => t('Last Name'), 'field' => 'a.last_name'),
    'email' => array('data' => t('E-Mail'), 'field' => 'a.email'),
    'visitor_type' => array('data' => t('Customer Type'), 'field' => 'a.visitor_type'),
    'market_focus' => array('data' => t('Industry'), 'field' => 'a.market_focus'),
    // 'country' => array('data' => t('Country'), 'field' => 'a.country'),
    'picture' => array('data' => t('Picture'),),
    'events' => array('data' => t('Events Registered For'), 'field' => 'a.events'),
    'created' => array('data' => t('Created'), 'field' => 'a.created', 'sort' => 'desc'),
    'operations' => array('data' => t('Operations'),)
  );
  $query = db_select('attendeeresource_attendees','a')
          ->extend('TableSort')
          ->extend('PagerDefault');
  $query->addTag('attendeeresource_admin_filter');
  attendeeresource_build_filter_query($query);
  $attendees = $query
              ->fields('a')
              ->limit(50)
              ->orderByHeader($header)
              ->execute()
              ->fetchAllAssoc('attendee_id');
  
  $options = array();
  foreach($attendees as $attendee) {
    $events = _attendeeresource_attendee_events($attendee->attendee_id);

    $operations = array();
    $link_options = array('query' => array('destination' => $destination['destination']));
    if (user_access('edit attendees')) {
      $operations[] = l('Edit', 'admin/structure/attendees/edit/' . $attendee->attendee_id);
      $operations[] = l('Delete', 'admin/structure/attendees/delete/' . $attendee->attendee_id, $link_options);
    }

    $options[$attendee->attendee_id] = array(
      'id' => $attendee->attendee_id,
      'first_name' => $attendee->first_name,
      'last_name' => $attendee->last_name,
      'email' => $attendee->email,
      'visitor_type' => $attendee->visitor_type,
      'market_focus' => $attendee->market_focus,
      // 'country' => _country_taxonomy_country($attendee->country)->name,
      'picture' => !empty($attendee->picture) ? l('View',file_create_url(file_load($attendee->picture)->uri),array('attributes' => array('rel' => 'shadowbox'))) : '--',
      'events' => count($events),
      //'created' => date('F j, Y g:i A T', $attendee->created),
      'created' => '<time class="timeago" datetime="' . date('c', $attendee->created) . '" title="' . date('c', $attendee->created) . '">' . date('F j, Y g:i A T', $attendee->created) . '</time>',
      'operations' => implode('&nbsp;&nbsp;&nbsp;',$operations),
    );
  }
  $form['pager'] = array('#markup' => theme('pager'));
  $form['attendees'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No Attendees exist.'),
  );
  $form['pager_bottom'] = array('#markup' => theme('pager'));
  return $form;
}

function attendeeresource_form_alter(&$form, $form_state, $form_id) {
  if ($form_id == 'attendeeresource_edit_form') {
    // Add country taxonomy handling
    $form['#after_build'][] = 'country_taxonomy_form_after_build';
  }
}

function attendeeresource_settings_submit($form, &$form_state) {
  switch($form_state['triggering_element']['#value']) {
    case 'Delete':
      $delete = array();
      foreach($form_state['values']['attendees'] as $attendee_id => $checked) {
        if ($checked !== 0) {
          $delete[] = $attendee_id;
        }
      }
      if (empty($delete)) {
        drupal_set_message(t('Please select at least one item.'),'error');
      }
      else {
        $deleted = _attendeeresource_delete_attendee_multiple($delete);
        drupal_set_message(t('@num attendees have been successfully deleted and removed from all events.',array('@num' => $deleted)));
      }
      break;
  }
}



function attendeeresource_edit_form($form, &$form_state, $attendee) {
  // Add shadowbox for picture previews
  drupal_add_library('shadowbox','shadowbox');

  $schema = drupal_get_schema('attendeeresource_attendees');
  foreach($schema['fields'] as $field_name => $field) {
    $friendly_field_name = ucwords(implode(' ', explode('_', $field_name)));
    // Logic based on type
    switch($field['type']) {
      case 'varchar':
        $form[$field_name] = array(
          '#type' => 'textfield', 
          '#title' => $friendly_field_name,
          '#default_value' => $attendee->{$field_name},
          '#required' => TRUE,
        );
        break;
    }
    // Logic based on name
    switch($field_name) {
      case 'attendee_id':
        $form[$field_name] = array('#type' => 'value', '#value' => $attendee->{$field_name});
      break;
      case 'visitor_type':
        $form[$field_name] = array(
          '#title' => $friendly_field_name,
          '#type' => 'select',
          '#options' => _eaton_backbone_visitor_types(),
          '#default_value' => $attendee->{$field_name},
          '#required' => TRUE,
        );
      break;
      case 'market_focus':
        $form[$field_name] = array(
          '#title' => $friendly_field_name,
          '#type' => 'select',
          '#options' => _eaton_backbone_market_focus(),
          '#default_value' => $attendee->{$field_name},
          '#required' => TRUE,
        );
      break;
      case 'country':
        $form[$field_name] = array(
          '#title' => $friendly_field_name,
          '#type' => 'select',
          '#options' => _country_taxonomy_country_options(),
          '#default_value' => $attendee->{$field_name},
          '#required' => TRUE,
        );
      break;
      case 'state':
        $form[$field_name] = array(
          '#title' => $friendly_field_name,
          '#type' => 'select',
          '#options' => _country_taxonomy_state_options(),
          '#default_value' => $attendee->{$field_name},
          '#required' => TRUE,
        );
      break;
      case 'picture':
        $form[$field_name] = array(
          '#title' => $friendly_field_name,
          '#type' => 'managed_file',
          '#upload_location' => 'public://attendee_pictures/',
          '#default_value' => $attendee->{$field_name},
          '#upload_validators' => array(
            'file_validate_extensions' => array('png jpg gif bmp jpeg'),
          ),
          // Attach custom theme function and JS to display file link in shadowbox
          '#theme' => 'attendeeresource_file_managed_file_shadowbox',
          '#attached' => array(
            'js' => array(
              'jQuery(document).ready(function() {  jQuery(\'div.form-managed-file .file a, .file-widget .file a\').click(function(e) { e.preventDefault(); e.stopPropagation(); }) })' => array(
                'type' => 'inline',
                )
            ),
          ),
        );
        break;
    }

    // Display Logic based on name
    switch($field_name) {
      case 'first_name':
      case 'company':
      case 'visitor_type':
      case 'email':
      case 'country':
      case 'zip':
        $form[$field_name]['#prefix'] = '<div style="float: left; clear: left; width: 47%; margin-right: 1.5%">';
        $form[$field_name]['#suffix'] = '</div>';
        break;
      case 'last_name':
      case 'title':
      case 'market_focus':
      case 'city':
      case 'state':
      case 'picture':
        $form[$field_name]['#prefix'] = '<div style="float: left; clear: right; width: 47%; margin-left: 1.5%">';
        $form[$field_name]['#suffix'] = '</div>';
        break;
    }
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
  $form['actions']['cancel'] = array('#type' => 'submit', '#value' => t('Cancel'));
  $form['#attributes']['class'][] = 'attendeeresource-edit-form';
  return $form;
}

function attendeeresource_edit_form_validate($form, &$form_state) {
  if (!form_get_errors()) {
    $attendee = _attendeeresource_get_attendee($form_state['values']['attendee_id']);
    $email = $form_state['values']['email'];
    if (strtolower($attendee->email) != strtolower($email) && _attendeeresource_lookup_attendee($email)) {
      form_set_error('email', t('Cannot use email address @email.  An attendee already exists in the system with this email address.', array('@email' => $email)));
    }
  }
}

function attendeeresource_edit_form_submit($form, &$form_state) {
  $destination = drupal_get_destination();
  switch($form_state['triggering_element']['#value']) {
    case 'Save':
      $values = $form_state['values'];
      $attendee_id = $form_state['values']['attendee_id'];

      // Unset keys in value array set by drupal because we are passing
      // the values array directly to the fields() method of the UpdateQuery
      foreach(array('save', 'delete', 'cancel', 'form_build_id', 'form_token', 'form_id', 'op') as $field_name) {
        if (isset($values[$field_name])) {
          unset($values[$field_name]);
        }
      }

      $attendee = _attendeeresource_get_attendee($attendee_id);
      $attendee_full_name = $attendee->first_name . ' ' . $attendee->last_name;
      if (isset($values['picture']) && !empty($values['picture']) && $values['picture'] != $attendee->picture) {
        // Picture updated/changed
        $file = file_load($values['picture']);
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);
        file_usage_add($file, 'attendeeresource', 'attendee' , $values['attendee_id']);
      }
      if (_attendeeresource_update_attendee($attendee_id, $values)) {
        drupal_set_message(t('Attendee <em>@name (@email)</em> has been succesfully updated.', array('@name' => $attendee_full_name, '@email' => $values['email'])));
      }
      else {
        drupal_set_message(t('An error occured while trying to update attendee <em>@name (@email)</em>.  Please try again later or contact site administrators.',array('@name' => $attendee_full_name, '@email' => $attendee->email)), 'error');
      }
    break;
    case 'Delete':
      $attendee_id = $form_state['values']['attendee_id'];
      drupal_goto('admin/structure/attendees/delete/' . $attendee_id, array('query' => $destination));
    break;
    case 'Cancel':
      drupal_goto('admin/structure/attendees');
    break;
  }
}

function attendeeresource_delete_form($form, &$form_state, $attendee) {
  $form['attendee_id'] = array('#type' => 'value', '#value' => $attendee->attendee_id);
  $form['title'] = array('#markup' => t('Are you sure you want to delete attendee <em>@name (@email)</em>?  <strong>This cannot be undone.</strong>',array('@name' => $attendee->first_name . ' ' . $attendee->last_name, '@email' => $attendee->email)));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['delete'] = array('#type' => 'submit', '#value' => 'Yes, delete this attendee');
  $form['actions']['cancel'] = array('#type' => 'submit', '#value' => 'Cancel');
  return $form;
}

function attendeeresource_delete_form_submit($form, &$form_state) {
  $attendee_id = $form_state['values']['attendee_id'];
  $attendee = _attendeeresource_get_attendee($attendee_id);
  switch($form_state['triggering_element']['#value']) {
    case 'Yes, delete this attendee':
      if (_attendeeresource_delete_attendee($attendee_id)) {
        drupal_set_message(t('Attendee <em>@name (@email)</em> has been successfully removed from the system.',array('@name' => $attendee->first_name . ' ' . $attendee->last_name, '@email' => $attendee->email)));
      }
      else {
        drupal_set_message(t('An error occured while trying to update attendee <em>@name (@email)</em>.  Please try again later or contact site administrators.',array('@name' => $attendee->first_name . ' ' . $attendee->last_name, '@email' => $attendee->email)), 'error');
      }
    break;
    case 'Cancel':
      drupal_goto('admin/structure/attendees/edit/' . $attendee_id);
    break;
  }
}