$(document).ready( function() {
  $("#full-node-version-testing-status-form .form-checkboxes .form-item input").click( function() {
    $checking = $(this).is(":checked");
    if ($checking) {
      $(this).parents(".form-item").siblings(".form-item").find("input").attr('checked', false);
    }
    else {
    }
  
  });
  $("input[value='disabled']").attr('disabled', true);
});
