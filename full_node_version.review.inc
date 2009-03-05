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
  /*
   */
        // if node is draft and has a parent then there are two options c and d
  //get all draft nodes and all non-superseded where testing and current do not match
  $nodes = full_node_version_get_nodes(array('is_draft'=>1));
  foreach(
    full_node_version_get_nodes(array('is_draft'=>0,'is_superseded'=>0,'is_testing'=>'!is_current' ))
    as $nid=>$n) { $nodes[$nid]= $n ;
  }
  foreach( $nodes as $nid=>$n) {
    $default  = array();
    $options = array();
    if ($n->fnv_parent_is_current || $n->fnv_is_current) {
      $image_string = theme('image',drupal_get_path("module", "full_node_version")."/tick.png");
    }
    else {
      $image_string = "";
    }
    if($n->fnv_is_draft && $n->fnv_revises) {
      $options = array("c"=> "", "d"=>"");
      if($n->fnv_is_testing) { $default = array('d'); }
      elseif($n->fnv_parent_is_testing) { $default = array('c'); }
    }
    elseif($n->fnv_is_draft) {
      $options = array("disabled"=> "", "d"=>"");
      if($n->fnv_is_testing) { $default = array('d'); }
      
    }
    else {
      $options = array("d"=> "", "disabled"=>"");
      if($n->fnv_is_testing) { $default = array('d'); }
    } 
    $form['testing_status']["$nid-wrapper"] = array (
      '#type' => 'fieldset',
    );
    # FIXME: Need to escape $n->title in the l function below?
    $form['testing_status']["$nid-wrapper"]["title"] = array(
      "#value" => t("<div class = 'form-checkboxes form-title'><div class= 'form-item page'>".l($n->title, 'node/'.$nid)."</div></div>"),
    );
    $form['testing_status']["$nid-wrapper"]["$nid"] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $default,
      '#attributes' => array('style' => 'clear:left'),
    );
    $form['testing_status']["$nid-wrapper"]["prod"] = array(
      '#value' => "
        <div class = 'form-checkboxes'>
          <div class = 'form-item double'>$image_string
          </div>
        </div>
      ",
    );
 
  }
  $form['nids'] = array(
    '#type' => 'hidden',
    '#value'=>  implode(',',array_keys($nodes)),
  );

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
  $nids  = explode(',',$values['nids']);
  foreach ($nids as $nid) {
    if ($s = $values[$nid]) {
      $n = node_load($nid);
      $n->fnv_is_testing        = (bool) $s['d'];
      $n->fnv_parent_is_testing = (bool) $s['c'];
      node_save($n);
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
