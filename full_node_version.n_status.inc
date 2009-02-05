

function full_node_version_status_html($node_in) {
  $type = $node_in->type;
  $full_node_version_node_types = variable_get('full_node_version_node_types',array());
  if(user_access('administer full node version on nodes') && 
       in_array($type, $full_node_version_node_types)) {
    $form = drupal_get_form('full_node_version_status_form'.$node_in->nid, $node_in);
    return $form;
  }
}

/*
 * This form is used on the view page to allow the publishing and unpublishing of 
 * the draft and current version of a page
 */
function full_node_version_status_form(&$form_state,$node) {
  $is_draft = full_node_version_is_status($node, 'draft');
  $current_nid = $is_draft ? $node->field_fnv_revises[0]["nid"] : $node->nid; 
  $current_node = $is_draft ? node_load($node->field_fnv_revises[0]["nid"]) : $node; 
  $draft_node = $is_draft   ? $node  : FALSE; 
  $draft_nid = $is_draft   ? $node->nid  : FALSE; 
  if ($current_nid == NULL) {}
  $cl_checked = full_node_version_is_status($current_node, 'current') ? "checked" : "";
  $ct_checked = full_node_version_is_status($current_node, 'testing')  ? "checked" : "";
  $dl_checked = full_node_version_is_status($node, 'current')         ? "checked" : "";
  $dt_checked = full_node_version_is_status($node, 'testing')         ? "checked" : "";

  if ($is_draft && $current_node) {
  $status_html = "
    <table>
      <tr><th></th><th>Draft</th><th>Published</th></tr>
      <tr>
        <th>In Production</th>
        <td><input class = 'fnv_status_option fnv-draft-live' $dl_checked item= 'fnv-draft-live' type ='checkbox' /></td>
        <td><input class = 'fnv_status_option fnv-current-live' $cl_checked item= 'fnv-current-live' type ='checkbox' /></td>
      </tr>
      <tr>
        <th>In Testing</th>
        <td><input class = 'fnv_status_option fnv-draft-test' $dt_checked item= 'fnv-draft-test' type ='checkbox' /></td>
        <td><input class = 'fnv_status_option fnv-current-test' $ct_checked item= 'fnv-current-test' type ='checkbox' /></td>
      </tr>
    </table>
    ";
  }
  elseif ($is_draft) {
    $status_html = "
    <table>
      <tr><th></th><th>Draft</th></tr>
      <tr>
        <th>In Production</th>
        <td><input class = 'fnv_status_option fnv-draft-live' $dl_checked item= 'fnv-draft-live' type ='checkbox' /></td>
        </tr>
      <tr>
        <th>In Testing</th>
        <td><input class = 'fnv_status_option fnv-draft-test' $dt_checked item= 'fnv-draft-test' type ='checkbox' /></td>
      </tr>
    </table>
    ";
  }
  else {
    $status_html = "
    <table>
      <tr><th></th><th>Current</th></tr>
      <tr>
        <th>Live</th>
        <td><input class = 'fnv_status_option fnv-current-live' $cl_checked item= 'fnv-current-live' type ='checkbox' /></td>
      </tr><tr>
        <th>Test</th>
        <td><input class = 'fnv_status_option fnv-current-test' $ct_checked item= 'fnv-current-test' type ='checkbox' /></td>
      </tr></table>
    ";
  }
  $form['div_tag'] = array('#type' => 'markup', '#value' => $status_html);
  
  $form['current_nid'] = array(
    '#type' => 'hidden',
    '#value' => $current_nid
  );
  $form['draft_nid'] = array(
    '#type' => 'hidden',
    '#value' => $draft_nid
  );
  // This field will be fill by js with a binary map of the chooses made on the page.
  $form['fnv_save_status'] = array(
    '#type' => 'textfield',
    '#attributes'=>array('style'=>"display:none", "class"=>"edit-fnv-save-status"),
    '#default_value' => '0'
  );
  $form["buttons"] = array (
    '#type' => 'fieldset',
    '#attributes'=>array('class'=>"buttons"),
  );
  $form["buttons"]['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
   // '#submit' => array("full_node_version_status_form_submit"=>array($node)),
  );
  $form["buttons"]['reset'] = array(
    '#type' => 'button',
    '#value' => t('Cancel'),
  );
  $form["#attributes"] = array("class" => "full-node-version-status-form");
  $form["#submit"] = array("full_node_version_status_form_submit1");
  return $form;
}

/*
 * On submit we process the saved status binary with the following layout
 * (Current is Live) (Current is test) (Draft Live) (Draft Test)
 */
function full_node_version_status_form_submit1($form,&$form_state) {
  //retrive values from node
  print "MADE IT MADE IT";
  $bvalue = $form_state["values"]["fnv_save_status"];
  $current_nid = $form_state["values"]["current_nid"];
  $draft_nid   = $form_state["values"]["draft_nid"];
  //provess the current node
  if ($current_nid) {
    if (($bvalue & 8) == 8) { //currentlive
      $current_node = node_load($current_nid, NULL, true);
      full_node_version_status_add($current_node, 'current');
    }
    else {
      $current_node = node_load($current_nid, NULL, true);
      full_node_version_status_remove($current_node, 'current');
    }
    if (($bvalue & 4) == 4) {//current test
      $current_node = node_load($current_nid, NULL, true);
      full_node_version_status_add($current_node, 'testing');
    }
    else {
      $current_node = node_load($current_nid, NULL, true);
      full_node_version_status_remove($current_node, 'testing');
    }
  }
  // if there is a draft node process it 
  if ($draft_nid) {
    if (($bvalue & 2) ==2) { //draft live
      full_node_version_status_remove($draft_nid, 'draft');
      _draft_to_current($draft_nid , $current_nid);
    }
    else {
      $draft_node = node_load($draft_nid, NULL, true);
      full_node_version_status_remove($draft_node, 'current');
    }
    if (($bvalue & 1) == 1) { //draft test
      $draft_node = node_load($draft_nid, NULL, true);
      full_node_version_status_add($draft_node, 'testing');
    }
    else {
      $draft_node = node_load($draft_nid, NULL, true);
      full_node_version_status_remove($draft_node, 'testing');
    }
  }
}


/*
 * This hook inserts the status form in the view of nodes with types being 
 * run by the full node version
 */
function full_node_version_nodeapi(&$node, $op) {
  
  //var_export(content_fields('field_fnv_revises', 'page'));
  $type = $node->type;
  $full_node_version_node_types = variable_get('full_node_version_node_types',array());
  // drupal_set_message(  ($op=='view' ) . "-" .       user_access('administer nodes') . "-".       user_access("edit any $type content") ."-".       in_array($type, $full_node_version_node_types)       );
  
  // // FIXME: This messes up our edit/published status, so disable for now
  // if (($op=='load' ) && 
  //      in_array($type, $full_node_version_node_types)
  //    ) {
  //   if (full_node_version_is_status($node,'superseded')) {
  //     // global $user;
  //     if( !node_access('view', $node) ) {
  //       $new_node = full_node_version_revised_by($node);
  //       DEBUG && drupal_set_message("Node " .  $node->nid . " has been superseded by node " . $new_node->nid . " " . $new_node->title);
  //       $node = $new_node;
  //     }
  //   }
  // }
  if (($op=='load' ) && 
       user_access('administer full node version on nodes') && 
       in_array($type, $full_node_version_node_types)
     ) {
    //return array("full_node_version_status_form_rendered"=>drupal_get_form('full_node_version_status_form', $node));
    //return array();
  }
  if (($op=='view' ) && 
       user_access('administer full node version on nodes') && 
       //user_access("edit any $type content") &&
       in_array($type, $full_node_version_node_types)
     ) {
    drupal_add_js(drupal_get_path('module', 'full_node_version').'/full_node_version_status_box.js', 'module');
    drupal_add_css(drupal_get_path('module', 'full_node_version').'/full_node_version_status_box.css', 'module');
    //$form = drupal_get_form('full_node_version_status_form', $node);
    //$node->content["status_form"]["#value"] = $form;
  }
  // // FIXME: Disabled this for new panels. See if it works.
  // if($op=='insert') {
  //   $parent_nid = $node->field_fnv_revises[0]["nid"];
  //   $parent_did = db_result(db_query("SELECT did from {panels_node} where nid = %d", $parent_nid));
  //   $child_did = $node->panels_node["did"];
  //   if ($child_did && $parent_did) {
  //     _full_node_version_new_draft_panel($child_did, $parent_did);   
  //   }
  // }
}
