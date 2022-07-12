(function($) {
  Drupal.behaviors.attendeeresource = {
    attach: function() {
      // Utilizing Drupal Backbone and Services API

      // ### Drupal.Backbone.Models.Attendee
      Drupal.Backbone.Models.Attendee = Drupal.Backbone.Models.Base.extend({
        urlRoot: "attendee",
        idAttribute: "attendee_id",

        initialize: function(opts) {
          Drupal.Backbone.Models.Base.prototype.initialize.call(this, opts);
        },

        toJSON: function() {
          return {
            attendee: Drupal.Backbone.Models.Base.prototype.toJSON.call(this)
          };
        },

        // Processor for Boolean values, needed due to way Services treats "false".
        // See http://drupal.org/node/1511662 and http://drupal.org/node/1561292
        toJSONBoolean: function(value) {
          if (value === 1 || value === "1" || value === true || value === "true") {
            return true;
          } else {
            return null;
          }
        },
        getName: function() {
            return this.get('first_name') + ' ' + this.get('last_name');
        },
        getLabel: function() {
            return this.getName() + ' (' + this.get('email') + ')';
        },
        dispatchEmail: function(restEndpoint,event_id,successCallback,errorCallback) {
            var attendee = this;
            if (errorCallback == undefined) {
                errorCallback = function() { console.log(arguments); alert(arguments); };
            }
            $.ajax({
                 url: restEndpoint + "/attendee/dispatch_email/" + event_id + '/' + attendee.get('attendee_id'),
                 type: "post",
                 dataType:"json",
                 error: errorCallback,
                 success: successCallback
             });
        },
        checkIn: function(restEndpoint,event_id,successCallback,errorCallback) {
          var attendee = this;
          if (errorCallback == undefined) {
              errorCallback = function() { console.log(arguments); alert(arguments); };
          }
          $.ajax({
               url: restEndpoint + "/attendee/check_in/" + attendee.get('attendee_id') + '/' + event_id,
               type: "post",
               dataType:"json",
               error: errorCallback,
               success: successCallback
           });
        },
        checkOut: function(restEndpoint,event_id,successCallback,errorCallback) {
          var attendee = this;
          if (errorCallback == undefined) {
              errorCallback = function() { console.log(arguments); alert(arguments); };
          }
          $.ajax({
               url: restEndpoint + "/attendee/check_out/" + attendee.get('attendee_id') + '/' + event_id,
               type: "post",
               dataType:"json",
               error: errorCallback,
               success: successCallback
           });
        }
      });

      // ### Drupal.Backbone.AttendeeIndexCollection
      //
      Drupal.Backbone.Collections.AttendeeIndex = Drupal.Backbone.Collections.Base.extend({
        model: Drupal.Backbone.Models.Attendee,
        url: function() {
          return this.restEndpoint + "/attendee.json";
        }
      });
      
      // ### Drupal.Backbone.Collections.EventAttendeesView
     Drupal.Backbone.Collections.EventAttendeesView = Drupal.Backbone.Collections.Base.extend({
        initialize: function(opts) {
	  opts = opts || {};
          this.constructor.__super__.initialize.call(this, opts);
          this.model = Drupal.Backbone.Models.Attendee;
          this.eventId = opts.eventId;
          this.sortKey = opts.sortKey ? opts.sortKey : 'last_name';
          this.sortOrder = opts.sortOrder ? opts.sortOrder : 'desc';
        },
        url: function() {
          return this.restEndpoint + "/event_attendees/" + this.eventId + ".json";
        },
        comparator: function (model) { 
          var output = 0;
          var val;
          val =  model.get(this.sortKey);
          if (typeof(val) != "undefined") {
            if (typeof(val.und) != "undefined") {
               output = val.und[0].value;
            } else {
               output = val;
            }   
          }
          if (output === false) { output = 0; }
          else if (output === true) { output = 1; }
          if (this.sortOrder == 'desc') {
              if (jQuery.isNumeric(output)) {
                  output = -output;
              } else {
                  str = output.toLowerCase();
                  output = output.split("");
                  output = _.map(output, function(letter) { 
                    return String.fromCharCode(-(letter.charCodeAt(0)));
                  });
              }
          }
          return output;
        },
        sortByField: function(key) {
          this.sortKey = key;
          this.sort();
          // trigger filter
          $("#filter-" + key).change();
        }
      });
    }
  };

})(jQuery);
