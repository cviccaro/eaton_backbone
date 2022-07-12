/*
 * This file, since it gets added after every ajax request done 
 * on the 'public_events' view table, simply adds a class to the 
 * active TH element to show a custom sorting arrow.
 */

(function($) {
    Drupal.behaviors.eaton_backbone_views_table = {
      attach: function(context, settings) {
          var $table = $("table.views-table");
          // add sorting class to default sort
          var $active = $('th.active',$table);
          var dir = $active.find('a').attr('href');
          var dir = typeof(dir) != "undefined" ? (dir.indexOf('sort=asc') == -1 ?  'asc' : 'desc') : '';
          $active.addClass('sorting ' + dir);
//          $(document).on('ajaxComplete',function(e,jqxhr,response) {
//              if (response.hasOwnProperty('data')) {
//                  var data = parseDataString(response.data);
//                  // just make sure we are acting on the (right) view ajax response
//                  if (data.hasOwnProperty('view_name') && data.view_name == 'public_events' && data.hasOwnProperty('order')) {
//                      var sort = data.sort;
//                      var order = data.order.split('_').join('-');
//                      $table.find('th.views-field-' + order).addClass('sorting ' + sort);
//                  }
//              }
//          });
          function parseDataString(str) {
              var parts = str.split('&');
              var data = {};
              for (var part, i = 0; part = parts[i]; i++) {
                  var keyval = part.split('=');
                  var key = decodeURIComponent(keyval[0]);
                  var val = decodeURIComponent(keyval[1]);
                  data[key] = val;
              }
              return data;
          }
      },
      unattach: function() {

      }
    };
})(jQuery);