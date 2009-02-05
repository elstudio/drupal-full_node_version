<?php
/*
 * This fuction builds a page to list all nodes of intererst 
 * and thier testing status.
 * nodes of interest are drafts and not drafts that are not suspended 
 * and not in testing
 */
function full_node_version_testing_status() {
  drupal_add_css(drupal_get_path('module','full_node_version') . "/full_node_version_testing_status.css", 'module');
  drupal_add_js(drupal_get_path('module','full_node_version') . "/full_node_version_testing_status.js", 'module');
  return drupal_get_form('full_node_version_testing_status_form');
}
/* build a form to list all nodes of intererst 
 * and thier testing status.
 * nodes of interest are drafts and not drafts that are not suspended 
 * and not in testing
 */
function full_node_version_testing_status_form($form_state) {
  $nodes = _full_node_version_status_of_nodes_array();
  $form['testing_status'] = array (
    '#type' => 'fieldset',
    '#title' => t('Versions in Testing'),
    '#description' =>t('Boxes checked indicate which version of a page is in testing, the draft version or the published version'),
  );
  $form['testing_publish'] = array (
    '#type' => 'fieldset',
    '#title' => t('Publishing Testing'),
    '#description' =>t('Publshing Testing moves the pages in testing to produciton.'),
  );
  $form['testing_status']["title"] = array(
    "#value" => "
    <div class = 'form-checkboxes form-title'>
    <div class = 'top-header'> </div><div class = 'header'>Page</div>
    </div>
    <div class = 'form-checkboxes'>
      <div class = 'top-header'>In Testing</div>
      <div class = 'form-item header '>Published</div>
      <div class = 'form-item header '>Draft</div>
    </div>
    <div class = 'form-checkboxes'>
      <div class = 'top-header'>In Production</div>
    </div>
    ",
  );
  foreach($nodes as $nid => $s) {
    $in_production = false;
    $n = node_load($nid);
    if ($s['d'] == 1) {
      $parent_nid = $n->field_fnv_revises[0]["nid"];
      if ($parent_nid > 0) {
        // if node is draft and has a parent then there are two options c and d
        $options = array("c"=> "", "d"=>"");
        if (full_node_version_is_status($parent_nid, 'current')) {
          // if node is draft and it has a parent that is current the set is_production
          $in_production = true;
        }
      }
      else {
        // if node is draft and has does not have a parent then there is only one option
        // but not that while it is d it is place in the published column
        $options = array("disabled"=> "", "d"=>"");
      }
      $parent_s = $nodes[$parent_nid];
      // set defaults if in testing d if parent in testing then c else empty
      if ($s['t'] == 1 ) {
        $default = array("d");
      }
      else if ($parent_s['t'] ==1) {
        $default = array("c");
      }
      else {
        $default = array();
      }
    }
    else if ($s['s'] ==0 && $s['t'] != $s['c']) {
      //if not a draft, not superseded and testing does not = current 
      //then there is one option of d 
      $options = array("c"=> "", "disabled"=>"");
      // set defaut if in testing 
      // if not in test (and there for in production) set in_production
      if ($s['t'] == 1 ) {
        $default = array("d");
      }
      else {
        $default = array();
        $in_production = true;
      }
    }
    // otherwise (not a draft or superseded or if not superseded testing = current) then skip
    else { continue;}

    $form['testing_status']["$nid-wrapper"]["title"] = array(
      "#value" => t("<div class = 'form-checkboxes form-title'><div class= 'form-item page'><a href = '/node/$nid'>".$n->title."</a></div></div>"),
    );
    $form['testing_status']["$nid-wrapper"]["$nid"] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $default,
      '#attributes' => array('style' => 'clear:left'),
    );

    $image_string = "<img src = '/".drupal_get_path("module", "full_node_version")."/tick.png'>";
    if (!$in_production) {
      $image_string = "";
    }
    $form['testing_status']["$nid-wrapper"]["prod"] = array(
      '#value' => "
        <div class = 'form-checkboxes'>
          <div class = 'form-item double'>$image_string
          </div>
        </div>
      ",
    );

  }

  $form['testing_status']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 50,
  );
  $form['testing_publish']['publish_testing'] = array(
    '#type' => 'submit',
    '#value' => t('Publish Testing'),
    '#submit' => array("full_node_version_testing_status_form_submit","full_node_version_publish_testing_submit"),
    '#weight' => -49,
  );
  return $form;

}

/*
 * this is the submit funciton for the form above, it changes the status of items
 * as the have been change on the form
 */
function full_node_version_testing_status_form_submit($node, &$form_state) {
  $values = $form_state['values'];
  $nodes = _full_node_version_status_of_nodes_array();
  foreach ($nodes as $nid => $s) {
    if ($s = $values[$nid]) {
      $n = node_load($nid);
      $parent_nid = $n->field_fnv_revises[0]["nid"];
      $parent_s = $nodes[$parent_nid];
      if ($s['d']) {
        full_node_version_status_add($nid, "testing");
      }
      else {
        full_node_version_status_remove($nid, "testing");
      }
      if ($s['c']) {
        full_node_version_status_add($parent_nid, "testing");
      }
      else {
        full_node_version_status_remove($parent_nid, "testing");
      }
    }
  }
  drupal_set_message("Testing Status Saved");

}
/* 
 * is the publish testing submit fuction, it takes all the nodes on the 
 * form and pass it to the publish_testing function
 */
function full_node_version_publish_testing_submit($node, &$form_state) {
  $nids = array_keys($form_state['values']);
  full_node_version_publish_testing($nids);
}
