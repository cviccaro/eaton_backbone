<div id="event-request-receipt-<?php print $node->nid;?>" class="event-request-receipt-wrapper">
  <!-- <h2 class="receipt-title">Thank you for requesting an event at your local Eaton Experience Center. Your event is currently being reviewed by the facility coordinator and you will receive information on your event’s acceptance or alternative date options shortly.</h2> -->
  <p class="description">
    To review information or make alterations, visit the <?php print l('Events page', 'events');?>.
     <div class="event-request-receipt-field event-request-receipt-field-interests-topics">
        <?php
$field = field_view_field('node', $node, 'field_interests_topics');
print str_replace('Event Description for Invite', 'Event Description', render($field));
?>
      </div>
  </p>
    <?php
$location = $node->field_event_location[LANGUAGE_NONE][0]['value'];
$timezone = eaton_backbone_timezone_for_location($location);
$status = $node->field_request_status[LANGUAGE_NONE][0]['value'];
$status_markup = ucwords($status);
if ($status == 'approved' || $status == 'denied') {
	$status_markup = l($status_markup, 'email/' . _eaton_backbone_encrypt_email_id($node->nid), array('absolute' => true, 'attributes' => array('target' => '_blank')));
}
?>
    <?php
$node_wrapper = entity_metadata_wrapper('node', $node);
if ($node_wrapper->field_register_initiator->value() == 1) {
	print '<p>You have opted to be registered for this event upon approval.  If approved, you will be the first registrant.</p>';
}
?>
  <div class="split-wrapper">
    <div class="split-left">
      <div class="event-request-receipt-field event-request-receipt-created"><h3 class="field-label">Scheduled</h3> <?php print format_date($node->created, 'medium', '', $timezone);?></div>
      <div class="event-request-receipt-field event-request-receipt-title"><h3 class="field-label">Event Name</h3> <?php print $node->title;?></div>
      <div class="event-request-receipt-field event-request-receipt-field-event-initiator">
        <?php
$field = field_view_field('node', $node, 'field_event_initiator');
print render($field);
?>
      </div>
      <div class="event-request-receipt-field event-request-receipt-field-event-initiator-email">
        <?php
$field = field_view_field('node', $node, 'field_event_initiator_email');
print render($field);
?>
      </div>
      <div class="event-request-receipt-field event-request-receipt-field-visitor-estimate">
        <?php $field = field_view_field('node', $node, 'field_visitor_estimate');
print render($field);?>
      </div>
        <?php
$display = array(
	'label' => 'above',
	'type' => 'date_default',
	'settings' => array(
		'format_type' => 'medium',
	),
);
?>
      <div class="event-request-receipt-field event-request-receipt-field-possible-dates">
        <?php
$field = field_view_field('node', $node, 'field_possible_dates', $display);
$rendered = render($field);
print str_replace('Possible Date', 'Event Date and Time', $rendered);
?>
      </div>
    </div>
    <div class="split-right">
      <!-- <div class="event-request-receipt-field event-request-receipt-field-purpose-of-visit">
        <?php //$field = field_view_field('node', $node, 'field_purpose_of_visit');
//print render($field);?>
      </div> -->
      <div class="event-request-receipt-field event-request-receipt-field-room-requirements">
        <?php
$field = field_view_field('node', $node, 'field_room_requirements');
print render($field);
?>
      </div>
        <?php if ($location == 'pgh'): ?>
         <div class="event-request-receipt-field event-request-receipt-field-psec-support">
            <?php
$field = field_view_field('node', $node, 'field_psec_support');
print render($field);
?>
         </div>
         <div class="event-request-receipt-field event-request-receipt-field-psec-support-length">
            <?php
$field = field_view_field('node', $node, 'field_psec_support_length');
print render($field);
?>
        </div>
            <?php if (!empty($node->field_psec_support_length) && is_array($node->field_psec_support_length) && isset($node->field_psec_support_length[LANGUAGE_NONE][0]) && $node->field_psec_support_length[LANGUAGE_NONE][0]['value'] == 'portion'): ?>
            <div class="event-request-receipt-field event-request-receipt-field-psec-support-portion">
                <?php
$field = field_view_field('node', $node, 'field_psec_support_portion');
print render($field);
?>
            </div>
            <?php endif;?>
        <div class="event-request-receipt-field event-request-receipt-field-psec-catering">
            <?php
$field = field_view_field('node', $node, 'field_psec_catering');
print render($field);
?>
        </div>
        <?php elseif ($location == 'hou'): ?>
         <div class="event-request-receipt-field event-request-receipt-field-additional-areas">
            <?php
$field = field_view_field('node', $node, 'field_additional_areas');
print render($field);
?>
         </div>
        <?php endif;?>
        <?php if (!empty($node->field_special_request)): ?>
        <div class="event-request-receipt-field event-request-receipt-field-special-request">
            <?php
$field = field_view_field('node', $node, 'field_special_request');
print render($field);
?>
        </div>
        <?php endif;?>
        <?php if (!empty($node->field_psec_support_provided_by)): ?>
        <div class="event-request-receipt-field event-request-receipt-field-psec-support-provided-by">
            <?php
$field = field_view_field('node', $node, 'field_psec_support_provided_by');
print render($field);
?>
        </div>
        <?php endif;?>
    </div>
    <a class="button register-button" href="<?php print url('register/' . $node->nid);?>" title="Private Registration Link">Private Registration Link</a>
  </div>

</div>
