<?php
/**
 * Events Request table to be used by Backbone
 * for Coordinators/Admins
 */
?>
<?php
// if (module_exists('eaton_elements')) {
//   $block = block_load('eaton_elements','eaton_event_table_switcher');
//   $block_render = _block_render_blocks(array($block));
//   $block_render['eaton_elements_eaton_event_table_switcher']->subject = '';
//   $b = _block_get_renderable_array($block_render);
//   print drupal_render($b);
// }
?>
<div id="eaton-event-requests-app" class="eaton-app coordinator">
  <form id="eaton-event-requests-app-filters" class="eaton-app-filters">
<!--     <div class="filter-field"><label for="filter-field_event_location">Filter by Location</label><select name="filter-field_event_location" id="filter-field_event_location"></select></div>
    <div class="filter-field"><label for="filter-field_request_status">Filter by Status</label><select name="filter-field_request_status" id="filter-field_request_status"></select></div>
    <div class="filter-field"><label for="filter-field_event_visibility">Filter by Visibility</label><select name="filter-field_event_visibility" id="filter-field_event_visibility"></select></div> -->
    <div class="filter-field"><label for="filter-field_possible_dates">Filter by Date:</label> <input type="text" class="datepicker" name="filter-field_possible_dates-min" id="filter-field_possible_dates-min" /> <div class="date-divider">-</div> <input type="text" class="datepicker" name="filter-field_possible_dates-max" id="filter-field_possible_dates-max" /></div>
  </form>
  <div id="eaton-event-requests-app-header" class="eaton-app-header">
    <div class="app-info">Viewing <span id="display-min">0</span> - <span id="display-max">0</span> of <span id="max-count">0</span> requests</div>
    <div class="app-config items-per-pager-container" id="eaton-event-requests-items-per-page-container"><label for="items_per_page">Items Per Page</label><select name="items_per_page" id="eaton-event-requests-items-per-page" class="eaton-app-items-per-page"></select></div>
    <div class="app-config table-refresh-container" id="eaton-event-requests-table-refresh-container"><label for="table_refresh">Refresh Rate</label><select name="table_refresh" id="eaton-event-requests-table-refresh" class="eaton-app-table-refresh"><<option value="0">Never</option><option value="30000">30s</option><option value="60000">60s</option><option value="120000">2m</option></select></div>
  </div>
  <table id="eaton-event-requests-container" class="eaton-app-table">
    <thead>
      <tr>
        <th class="views-field views-field-created sorting desc"><a href="#" title="sort by Scheduled" class="active" data-key="created">Scheduled</a></th>
        <th class="views-field views-field-title"><a href="#" title="sort by Event Name" data-key="title">Event Name</a></th>
        <!-- <th class="views-field views-field-event-location"><a href="#" title="sort by Event Location" data-key="field_event_location">Location</a></th> -->
<!--        <th class="views-field views-field-visitor-type"><a href="#" title="sort by Visitor Type" data-key="field_visitor_type">Visitor Type</a></th>-->
        <!-- <th class="views-field views-field-event-visibility"><a href="#" title="sort by Visibility" data-key="field_event_visibility">Visibility</a></th> -->
        <th class="views-field views-field-uid"><a href="#" title="sort by Initiator" data-key="uid">Initiator</a></th>
<!--        <th class="views-field views-field-customer-boolean"><a href="#" title="sort by Potential or Existing Customer" data-key="field_customer_boolean">Customer</a></th>-->
        <th class="views-field views-field-field-possible-dates-1"><a href="#" title="sort by Date" data-key="field_possible_dates_1">Date</a></th>
        <!-- <th class="views-field views-field-field-possible-dates-2"><a href="#" title="sort by Possible Date 2" data-key="field_possible_dates_2">Possible Date 2</a></th>
        <th class="views-field views-field-field-possible-dates-3"><a href="#" title="sort by Possible Date 3" data-key="field_possible_dates_3">Possible Date 3</a></th> -->
        <!-- <th class="views-field views-field-field-denied-suggested-date"><a href="#" title="sort by Denied Suggested Date" data-key="field_denied_suggested_date">Suggested Date (if Denied)</a></th> -->
        <!-- <th class="views-field views-field-request-status"><a href="#" title="sort by Status" data-key="field_request_status">Status</a></th> -->
        <th class="views-field views-field-updated"><a href="#" title="sort by Last Updated" data-key="changed">Last Updated</a></th>
        <th class="views-field views-field-operations">Operations</th>
     </tr>
    </thead>
    <tbody>
      <tr><td colspan="10">Loading.. please wait.</td></tr>
    </tbody>
  </table>
  <nav id="eaton-event-requests-app-pager" class="eaton-pager"></nav>
</div>
<div id="modal"></div>
<div id="suggested-denial-date-form" style="display:none;">
  <form id="deny_event_request_form">
 <div class="field-type-datetime field-name-field-denied-suggested-date field-widget-date-popup form-wrapper" id="edit-field-denied-suggested-date">
 <div id="field-denied-suggested-date-add-more-wrapper">
 <fieldset class="form-wrapper">
 <legend>
 <span class="fieldset-legend">Please suggest a new date and time </span>
 </legend>
 <div class="fieldset-wrapper">
<div class="date-no-float start-date-wrapper container-inline-date"><div class="form-item form-type-date-popup form-item-field-denied-suggested-date-und-0-value">
 <div id="edit-field-denied-suggested-date-und-0-value" class="date-padding"><div class="form-item form-type-textfield form-item-field-denied-suggested-date-und-0-value-date">
  <label for="edit-field-denied-suggested-date-und-0-value-datepicker-popup-0">Date </label>
 <input class="date-clear form-text" type="text" id="edit-field-denied-suggested-date-und-0-value-datepicker-popup-0" name="field_denied_suggested_date[und][0][value][date]" value="" size="20" maxlength="30">
<div class="description"> E.g., 2014-06-12</div>
</div>
<div class="form-item form-type-textfield form-item-field-denied-suggested-date-und-0-value-time">
  <label for="edit-field-denied-suggested-date-und-0-value-timeEntry-popup-1">Time </label>
 <input class="date-clear form-text" type="text" id="edit-field-denied-suggested-date-und-0-value-timeEntry-popup-1" name="field_denied_suggested_date[und][0][value][time]" value="" size="15" maxlength="10">
<div class="description">E.g., 23:45</div>
</div>
</div>
</div>
</div><div class="date-no-float end-date-wrapper container-inline-date"><div class="form-item form-type-date-popup form-item-field-denied-suggested-date-und-0-value2">
  <label for="edit-field-denied-suggested-date-und-0-value2">to: </label>
 <div id="edit-field-denied-suggested-date-und-0-value2" class="date-padding"><div class="form-item form-type-textfield form-item-field-denied-suggested-date-und-0-value2-date">
  <label for="edit-field-denied-suggested-date-und-0-value2-datepicker-popup-0">Date </label>
 <input class="date-clear form-text" type="text" id="edit-field-denied-suggested-date-und-0-value2-datepicker-popup-0" name="field_denied_suggested_date[und][0][value2][date]" value="" size="20" maxlength="30">
<div class="description"> E.g., 2014-06-12</div>
</div>
<div class="form-item form-type-textfield form-item-field-denied-suggested-date-und-0-value2-time">
  <label for="edit-field-denied-suggested-date-und-0-value2-timeEntry-popup-1">Time </label>
 <input class="date-clear form-text" type="text" id="edit-field-denied-suggested-date-und-0-value2-timeEntry-popup-1" name="field_denied_suggested_date[und][0][value2][time]" value="" size="15" maxlength="10">
<div class="description">E.g., 23:45</div>
</div>
</div>
</div>
</div></div></fieldset>
</div></div>
  </form>
</div>

<div id="denial-message-form" class="panel-pane pane-entity-form-field pane-node-field-denied-message" style="display:none">
  <div class="pane-content">
    <form>
<div class="field-type-text-long field-name-field-denied-message field-widget-text-textarea form-wrapper" id="edit-field-denied-message"><div id="field-denied-message-add-more-wrapper"><div class="form-item form-type-textarea form-item-field-denied-message-und-0-value">
  <label for="edit-field-denied-message-und-0-value">Additional Suggestions </label>
 <div class="form-textarea-wrapper resizable textarea-processed resizable-textarea"><textarea class="text-full form-textarea" id="edit-field-denied-message-und-0-value" name="field_denied_message[und][0][value]" cols="60" rows="5"></textarea><div class="grippie"></div></div>
</div>
</div></div>
    </form>
  </div>
  </div>