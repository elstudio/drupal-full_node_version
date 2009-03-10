<?php 
// $Id: full_node_version.node_status.inc,v 1.1 2009/03/10 18:25:31 elstudio Exp $

/**
 * access function for the full_node_version node status
 * @return 
 *   a bool true if the node type is under full node version and the user has $access
 */
function full_node_version_node_status_access($access, $nid) {
  $node = node_load($nid);
  $fnv_types =  variable_get('full_node_version_node_types', array());
  return (in_array($node->type, $fnv_types)  && user_access($access));

}

/**
 * creats full node version form
 */
function full_node_version_node_status($nid) {
  $node = node_load($nid);
  return drupal_get_form('full_node_version_node_status_form', $node);
}

/**
 * full node version node status form
 */
function full_node_version_node_status_form($form_state, $node) {
  $form['#node'] = $node;
  $testing_default=array();
  $testing_options=array();
  if ($node->fnv_is_draft && $node->fnv_revises) {
    $testing_options = array("c" => "Current", "d" => "Draft");
    if ($node->fnv_is_testing) {
      $testing_default = array('d'); 
    }
    elseif ($node->fnv_parent_is_testing) { 
      $testing_default = array('c'); 
    }
  }
  elseif ($node->fnv_is_draft) {
    $testing_options = array("d" => "Draft");
    if ($node->fnv_is_testing) { 
      $testing_default = array('d'); 
    }
    
  }
  else {
    $testing_options = array("d" => "Current");
    if ($node->fnv_is_testing) { 
      $testing_default = array('d'); 
    }
  } 


  $form['testing_status'] = array(
    '#title'  => 'In Testing',
    '#type' => 'checkboxes',
    '#options' => $testing_options,
    '#default_value' => $testing_default,
  );
  $live_default=array();
  $live_options=array();
  if ($node->fnv_is_draft && $node->fnv_revises) {
    $live_options = array("c" => "Current", "d" => "Draft");
    if ($node->fnv_is_curent) { 
      $live_default = array('d'); 
    }
    elseif ($node->fnv_parent_is_current) { 
      $live_default = array('c'); 
    }
  }
  elseif ($node->fnv_is_draft) {
    $live_options = array("d" => "Draft");
    if ($node->fnv_is_current) {
      $live_default = array('d'); 
    }
  }
  else {
    $live_options = array("d" => "Current");
    if ($node->fnv_is_current) { 
      $live_default = array('d'); 
    }
  } 


  $form['live_status'] = array(
    '#title'  => 'Live',
    '#type' => 'checkboxes',
    '#options' => $live_options,
    '#default_value' => $live_default,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 50,
  );
  
  return $form;
}

/**
 * full node version node status form validation
 * check to insure that with both testing and live at most one item is selected
 */
function full_node_version_node_status_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (
    $values['testing_status']['d'] &&
    $values['testing_status']['c']
    ) {
    form_set_error('testing_status', t('Only one version can be in testing'));
    $error = TRUE;
  }
  if (
    $values['live_status']['d'] &&
    $values['live_status']['c']
    ) {
    form_set_error('live_status', t('Only one version can be live'));
    $error = $true;
  }
  return !$error;

}

/**
 * full noe verison node status form submit
 */
function full_node_version_node_status_form_submit($form, &$form_state) {
  $node = $form['#node'];
  $values = $form_state['values'];
  $testing = $form_state['values']['testing_status'];
  $live    = $form_state['values']['live_status'];

  $node->fnv_is_testing        = (bool) $testing['d'];
  $node->fnv_parent_is_testing = (bool) $testing['c'];
  
  $node->fnv_is_current        = (bool) $live['d'];
  $node->fnv_parent_is_current = (bool) $live['c'];
  if ($node->fnv_is_draft && $live['d']) {

    $node->fnv_is_draft = 0;
    $node->fnv_is_testing        = 1;
    $node->fnv_parent_is_testing = 0;
  }
  node_save($node);
  drupal_set_message(t('Status updated'));
}

