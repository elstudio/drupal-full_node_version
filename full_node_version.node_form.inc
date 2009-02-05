<?php
/**
 * Implements hook_form_alter()
 * Here forms for nodes that have been selected in the admin section are modified
 * If the form is not marked working (field_fnv_status ='working') then the new form 
 * is a create form with all of the information from the old node
 */
function full_node_version_form_alter(&$form, $form_state, $form_id) {
  // capture the node types that have fnv applied and then map them to form_ids 
  $full_node_version_form_ids = 
    array_map(
      create_function('$a','return $a."_node_form";'),
      variable_get('full_node_version_node_types',array())
    );

  // Only operate on node types that are under the control of fnv
  if (array_search( $form_id,$full_node_version_form_ids) !== FALSE) {
    $node = $form["#node"];
    $type = $form["type"]["#value"];


    //START create Status form item
    
    //if node is testing and not current then make as testing else mark as draft
    $status_default = 'make_draft';
    if ($node->field_fnv_status) {
      $status_array = array_map( create_function('$a',' return $a["value"];'),$node->field_fnv_status );
    }
    else {
      $status_array = array();
    }
    if ( 
      in_array('testing',$status_array) &&
      !in_array('current',$status_array)
    ) {
      $status_default = 'make_testing';  
    }
    if( user_access("administer full node version on nodes") ){
      $status_options = array( 
        'make_draft' => 'Draft', 
        'make_testing' => 'In Testing', 
      );
      $form['buttonstop']['fnv_flow_status'] = array(
        '#type' => 'radios',
        '#title' => t('status'),
        '#options' => $status_options,
        '#default_value' => $status_default,
        '#weight' => -50,
      );
    }
    //END create Status form item
 
    // check to see if this is a draft node
    if (
      ($node->field_fnv_status) &&
      (array_search(
        'draft',
        array_map( create_function('$a',' return $a["value"];'),$node->field_fnv_status )
      ) !== FALSE) 
    ) {
      //remove draft from path name

      $form['buttonstop']['fnv_flow_status']['#options']['make_live'] = 'In Production'; 

      $form["path"]["path"]["#default_value"] =  ereg_replace  ( "\_\(draft\)$"  ,"" ,  $form["path"]["path"]["#default_value"]);
      //drupal_set_message("This is a update form");
      //store the fnv_flow_status so we can see if it changes
      $form['fnv_flow_status_previous'] = array(
        '#type' => 'hidden',
        '#title' => t('status'),
        '#options' => $status_options,
        '#default_value' => $status_default,
      );
    }
    else {
      //drupal_set_message("This is a new creation form");
      //set not to process on drupal_retrieve_form

      if (!$form["#node"]->new) {
        // set the revises field
        $form['field_fnv_revises'][0]["#default_value"] = array("nid"=> $node->nid);

        if($node->type == "panel") {
          /*
          $addnode = $node;
          $addnode->nid = false;
          $addnode->path = $addnode->path ."_(draft)";
          $addnode->field_fnv_revises = array(array("nid"=>$node->nid));
          $addnode->panel_layout = "flexible";
          $addnode->field_fnv_status = array(array("value"=>"draft"));
          $addnode->new=true;
          */
          //print node_save($addnode);
          $addnode = array('uid' => $user->uid, 'name' => $user->name, 'type' => $type, 'new' =>true, "panel_layout" =>"flexible");
          $addform = drupal_retrieve_form('page_node_form', $form_state, $addnode);
          $form["panels_node"]["layout"] = $addform["panels_node"]["layout"];
        
        }
        else {
          $addnode = array('uid' => $user->uid, 'name' => $user->name, 'type' => $type, 'new' =>true);
          $addform = drupal_retrieve_form('page_node_form', $form_state, $addnode);
        }
          // for all fields that start with a # (all attrabute fields) copy the ones from the new
          // $addform form
          // also copy over the nid item
          foreach ($addform as $key => $item) {
            if (
                 preg_match("/^#/",$key) 
              || $key == "nid"
              ) {
              $form[$key] = $item;
            }
          }
          // to clear the menu we must copy form menu #ite and menu mlid
          $form["menu"]["#item"] = $addform["menu"]["#item"];
          $form["menu"]["mlid"] = $addform["menu"]["mlid"];
        
      }
    }
    // modifify the delete object if we have a parent.
    if ($form['field_fnv_revises'][0]['#default_value']["nid"] ) {
      drupal_add_js('
      $(document).ready(function() {
        $("#edit-pathauto-perform-alias").click();
        $("#edit-path").attr("disabled",false)
        })'
      , "inline");
    
    }
    
    if ( ($form['buttons']['delete']) ) {
      array_unshift($form['buttons']['delete']["#submit"],"full_node_revision_node_revert");
      $form['buttons']['delete']["#value"] = "Delete Draft";
      $form['buttonstop']['delete']    = $form['buttons']['delete'] ;
    }
    $form['buttonstop']['#weight'] = -100;
    $form['buttonstop']['#type'] = 'fieldset';
    $form['buttonstop']['#attributes'] = array('class'=>"full_node_version_status");
    $form['buttonstop']['submit'] = $form['buttons']['submit'] ;
    $form['buttonstop']['preview']   = $form['buttons']['preview'];

    //add our submit function
    array_unshift($form["#validate"],"full_node_revision_node_validate");
    array_push($form["#submit"],"full_node_revision_node_submit");
  global $user;
  $user->view_superseded = 1;
  }
}
/*
 * This function checks checks the fnv_flow_status and change the field_fnv_status before the 
 * form is processed.
 */
function full_node_revision_node_validate($form, &$form_state) {

  global $user;
  $user->view_superseded = 1;
  //Append Draft to end of path if we have a parent record and  there are no paths like it.
  if ($parent_nid = $form_state['values']['field_fnv_revises'][0]["nid"]) {
    //form_set_value($form["pathauto_perform_alias"],array("value"=>0), $form_state);
    $src = "node/$parent_nid";
    $dst =  urlencode($form_state["values"]["path"]);
    if (db_fetch_array(db_query("SELECT pid FROM {url_alias} WHERE src != '%s' and dst = '%s' " , $src, $dst))) {
      drupal_set_message("Path already used", 'error');
      return FALSE;
    }
    else {
    form_set_value($form["path"]["path"],$form_state["values"]["path"] . "_(draft)", $form_state);
    }
  }
  $status_action = $form_state['values']['fnv_flow_status'];
  form_set_value($form["field_fnv_status"],array(array("value"=>"draft")), $form_state);
  $status_action_p = $form_state['values']['fnv_flow_status_previous'];
  if ($status_action == $status_action_p) { $status_action = "none";}
  switch($status_action) {
    case "make_draft":
      form_set_value($form["field_fnv_status"],array(array("value"=>"draft")), $form_state);
    break;
    case "make_testing" :
      form_set_value($form["field_fnv_status"],array(array("value"=>"draft"), array("value"=>"testing")), $form_state);
    break;
    case "make_live":

      form_set_value($form["field_fnv_status"],array(array("value"=>"current"),array("value"=>"testing")), $form_state);
    break;
  }
}
/** 
 * This function is the submit function for fnv_node_form
 * It purpose is to process changes to the parent node status
 */
function full_node_revision_node_submit($form, &$form_state) {
  $status_action = $form_state['values']['fnv_flow_status'];
  $status_action_p = $form_state['values']['fnv_flow_status_previous'];
  if ($status_action == $status_action_p) { $status_action = "none";}
  $parent_nid   = $form_state['values']['field_fnv_revises'][0]["nid"];
  switch($status_action) {
    case "make_draft":
    // set parent to testing
      full_node_version_status_add($parent_nid, "testing");
    break;
    case "make_testing" :
    // remove parent testing
      full_node_version_status_remove($parent_nid, "testing");
    break;
    case "make_live":
    // set parent to not current 
      _draft_to_current($form["nid"]["#value"] , $parent_nid);

    break;
  }
  if ($form["nid"]["#value"] == NULL) {
    full_node_version_status_add($parent_nid, "superseded");
  }
}
/*
 * This function change the status of the parent node before the child is removed
 * removing the superseded status and seting the testing status if the draft was 
 * testing
 */
function full_node_revision_node_revert($form, &$form_state) {

  $parent_nid   = $form_state['values']['field_fnv_revises'][0]["nid"];

  full_node_version_status_remove($parent_nid, "superseded");
  if (full_node_version_is_status($form['#node'], 'testing')) {
    full_node_version_status_add($parent_nid, "testing");
  }

}
