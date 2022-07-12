(function($) {
    Drupal.behaviors.eaton_badge_links = {
        attach: function(context, settings) {
            $(document).on('click','a.badge-link',function(e) {
                        e.preventDefault();
                        e.stopPropagation();
                        if (!$("#modal").length) {
                            $("body").append('<div id="modal" />');
                        }
                        loadingIndicator.init();
                        
                        var previous_text = $("#working-text").text();
                        $("#working-text").text('Building badge...  Please be patient.');

                        // Load URL from href into dialog
                        var url             = $(e.target).attr('href'),
                            $tr             = $(e.target).parents('tr'),
                            attendee_name   = $tr.find('.field-name').text(),
                            opts            = {
                                width: 760,
                                height:530,
                                title: 'Badge for ' + attendee_name,
                                 autoOpen: false,
                                show: {effect: 'blind', duration: 300},
                                hide: {effect: 'blind', duration: 300},
                                'buttons':[
                                    {
                                        'text': 'Print This Badge',
                                        'click': function() {
                                            // Open new window and print from it
                                            var printWindow = window.open(url + '?pdf=1','','width=700, height=400');
                                            printWindow.onload = function() {  setTimeout(function() { printWindow.document.close(); printWindow.focus(); printWindow.print() },1000);}
                                        },
                                    },
                                    {
                                      'text': 'Close',
                                      'click': function() { 
                                          $(this).dialog('close'); 
                                      }
                                    },
                                ],
                                open: function() {
                                    var button = $(".ui-dialog-buttonpane").find('button:eq(0)');
                                    button.addClass('print-button').prepend('<span class="icon icon-print">&nbsp;</span>');
                                },
                                close: function() {
                                    $("#working-text").text(previous_text);
                                }
                            },
                            dialog = $("#modal").dialog(opts);
                            
                            $.get(url,function(data) {
                                //dialog.html(data);
                                // Use google's PDF viewer to embed PDF.
                                //dialog.html('<iframe class="mdocs-google-doc" src="//drive.google.com/viewer?url=' + url + '&embedded=true" style="width:100%; height:100%;" frameborder="0"></iframe>');
                                //dialog.html('<iframe class="mdocs-google-doc" src="' + url + '" style="width:100%; height:100%;" frameborder="0"></iframe>');
                                
                                // New solution.  PDFObject.js
                                // @link www.pdfobject.com
                                // dialog.html('<iframe style="width: 100%; height: 100%" frameborder="0"><div id="pdf_iframe"><p>It appears you don\'t have Adobe Reader or PDF support in this web browser. <a href="' + url + '">Click here to download the PDF</a></p></div></iframe>');
                                // dialog.dialog('option', 'open', function() {
                                //     var pdf = new PDFObject({url: url});
                                //     pdf.embed('pdf_iframe');
                                // })
                                var absurl = url;
                                if (url.indexOf('http://') == -1) {
                                    absurl = window.location.protocol + '//' + window.location.hostname + url;
                                }
                                var dialog_html = '<object data="' + absurl + '" type="application/pdf" width="100%" height="100%"><embed allowscriptaccess="always" src="' + absurl + '" width="100%" height="100%" alt="pdf" pluginspage="http://www.adobe.com/products/acrobat/readstep2.html" /></object>';
                                dialog.html(dialog_html);
                                dialog.dialog('open');
                            });
                    });
        }
    }
})(jQuery);