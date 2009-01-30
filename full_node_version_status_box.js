
$(document).ready( function() {

  $("input.fnv_status_option").click(full_node_version_select_option);
  $("#edit-reset").click( function() {
    $(this).parents("form")[0].reset();

  });
  $("form#full-node-version-status-form").find("input[type='submit']").attr('disabled', true);

});
function full_node_version_select_option() {
  
  checking = $(this).is(":checked");
  form = $(this).parents("form.full-node-version-status-form");
  switch($(this).attr("item") ){
    case "fnv-draft-live":
      if (checking) {
        $(form).find(".fnv-current-test").attr('checked', false).attr('disabled', true);
        $(form).find(".fnv-current-live").attr('checked', false).attr('disabled', true);
        $(form).find(".fnv-draft-test").attr('checked', true).attr('disabled', true);
        $(form).find(".fnv-draft-live").attr('checked', true).attr('disabled', true);
      }
      break;    
    case "fnv-draft-test":
      if (checking) {
        $(form).find(".fnv-current-test").attr('checked', false);
      }
      break;    
    case "fnv-current-live":
      if (checking) {
        $(form).find(".fnv-draft-live").attr('checked', false);
      }
      break;    
    case "fnv-current-test":
      if (checking) {
        $(form).find(".fnv-draft-test").attr('checked', false);
      }
      break;    
  }
  $(form).find("input[type='submit']").attr('disabled', false);
  var b = 0;
  if ( $(form).find(".fnv-current-live").is(":checked") ) {b+=8;}
  if ( $(form).find(".fnv-current-test").is(":checked") ) {b+=4;}
  if ( $(form).find(".fnv-draft-live").is(":checked") ) {b+=2;}
  if ( $(form).find(".fnv-draft-test").is(":checked") ) {b+=1;}
 
  $(form).find(".edit-fnv-save-status").val(b);
}
/*
function full_node_version_submit_status() {
  $("#edit-current-test").val($("#fnv-current-test").val());
  $("#edit-current-live").val($("#fnv-current-live").val());
  $("#edit-draft-test").val($("#fnv-draft-test").val());
  $("#edit-draft-live").val($("#fnv-defat-live").val());
}
*/
