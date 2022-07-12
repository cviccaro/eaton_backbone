<?php
/**
 * Table holding event attendees for backbone
 */
?>

<div id="eaton-event-attendees-app" class="eaton-app grey-background" data-id="<?php print $nid;?>">
  <h2>Current Registrants</h2>
<!--  <form id="eaton-event-requests-app-filters">
    <div class="filter-field"><label for="filter-field_event_location">Filter by Location</label><select name="filter-field_event_location" id="filter-field_event_location"></select></div>
    <div class="filter-field"><label for="filter-field_request_status">Filter by Status</label><select name="filter-field_request_status" id="filter-field_request_status"></select></div>
  </form>-->
  <div id="eaton-event-attendees-app-header" class="eaton-app-header">
    <div class="app-info">Viewing <span id="display-min">0</span> - <span id="display-max">0</span> of <span id="max-count">0</span> registrants</div>
    <div class="app-config items-per-pager-container" id="eaton-event-attendees-items-per-page-container"><label for="items_per_page">Items Per Page</label><select name="items_per_page" id="eaton-event-attendees-items-per-page" class="eaton-app-items-per-page"></select></div>
    <div class="app-actions">
      <div style="margin-bottom:10px; height: 15px"><input name="rebuild_badges" id="rebuild_badges" type="checkbox" /><label style="margin-left: 5px" for="rebuild_badges">Rebuild Badges when Viewing</label></div>
      <?php print l('View All Badges - Front', 'badges/' . $nid, array('attributes' => array('target' => '_blank', 'id' => 'view_all_badges--front', 'class' => array('button'))));?>
      <?php //print l('View All Badges - Back','badges/' . $nid . '/back', array('attributes' => array('target' => '_blank', 'id' => 'view_all_badges--back', 'class' => array('button')))); ?>
      <?php print l('Send All Emails', '#', array('attributes' => array('id' => 'send_all_emails', 'class' => array('button'))));?>
      <?php print l('Check In All', '#', array('attributes' => array('id' => 'check_in_all', 'class' => array('button'))));?>
      <?php print l('Check Out All', '#', array('attributes' => array('id' => 'check_out_all', 'class' => array('button'))));?>
    </div>
  </div>
  <table id="eaton-event-attendees-container" class="eaton-app-table">
    <thead>
      <tr>
       <th class="views-field view-term-id" style="display:none">ID</th>
       <th class="views-field views-field-field-attendee-last-name"><a href="#" title="sort by Name" class="active" data-key="last_name">Full Name</a></th>
       <th class="views-field views-field-attendee-email"><a href="#" title="sort by Email" class="active" data-key="email">Email</a></th>
       <th class="views-field views-field-date-registered sorting desc"><a href="#" title="sort by Date Registered" class="active" data-key="timestamp">Date Registered</a></th>
       <th class="views-field views-field-checked-in"><a href="#" title="sort by Check In/Out" class="active" data-key="check_status">Checked In/Out</a></th>
       <th class="views-field views-field-operations">Operations</th>
     </tr>
    </thead>
    <tbody>
      <tr id="empty-row"><td colspan="5">No one has been registered for this event.</td></tr>
    </tbody>
  </table>
<nav id="eaton-event-requests-app-pager" class="eaton-app-pager"></nav>
</div>

<div id="modal"></div>