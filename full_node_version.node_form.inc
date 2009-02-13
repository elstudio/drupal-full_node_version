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
    //grab current node fnv info
    $form['fnv_revises']       = array('#type' =>'hidden', '#value'=>$node->fnv_revises);
    $form['fnv_is_draft']      = array('#type' =>'hidden', '#value'=>$node->fnv_is_draft);
    $form['fnv_is_testing']    = array('#type' =>'hidden', '#value'=>$node->fnv_is_testing);
    $form['fnv_is_current']    = array('#type' =>'hidden', '#value'=>$node->fnv_is_current);
    $form['fnv_is_superseded'] = array('#type' =>'hidden', '#value'=>$node->fnv_is_superseded);
    $form['fnv_revised_by']       = array('#type' =>'hidden', '#value'=>$node->fnv_revise_by);
    $form['fnv_parent_is_draft']      = array('#type' =>'hidden', '#value'=>$node->fnv_parent_is_draft);
    $form['fnv_parent_is_testing']    = array('#type' =>'hidden', '#value'=>$node->fnv_parent_is_testing);
    $form['fnv_parent_is_current']    = array('#type' =>'hidden', '#value'=>$node->fnv_parent_is_current);
    $form['fnv_parent_is_superseded'] = array('#type' =>'hidden', '#value'=>$node->fnv_parent_is_superseded);

    //START create Status form item
    
    //if node is testing and not current then make as testing else mark as draft
    if ($node->fnv_is_testing &&!$node->fnv_is_current) {
      $status_default = 'make_testing';  
    }
    else {
      $status_default = 'make_draft';
    }
    // create top button if perm allow
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
    if ($node->fnv_is_draft) {
      //added option to go to production
      $form['buttonstop']['fnv_flow_status']['#options']['make_live'] = 'In Production'; 

      //remove draft from path name
      $form["path"]["path"]["#default_value"] =  ereg_replace  ( "\_\(draft\)$"  ,"" ,  $form["path"]["path"]["#default_value"]);
      //store the fnv_flow_status so we can see if it changes
      $form['fnv_flow_status_previous'] = array(
        '#type' => 'hidden',
        '#value' => $status_default,
      );
    }
    else { //node was not a draft
      $form['fnv_parent_is_draft']      = array('#type' =>'hidden', '#value'=>$node->fnv_is_draft);
      $form['fnv_parent_is_testing']    = array('#type' =>'hidden', '#value'=>$node->fnv_is_testing);
      $form['fnv_parent_is_current']    = array('#type' =>'hidden', '#value'=>$node->fnv_is_current);
      $form['fnv_parent_is_superseded'] = array('#type' =>'hidden', '#value'=>$node->fnv_is_superseded);

      //set not to process on drupal_retrieve_form
      if (!$form["#node"]->new) {
        // set the revises field
        $form['fnv_revises'] = array('#type' =>'hidden', '#value'=>$node->nid);

        if($node->type == "panel") {
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
        // to clear the menu we must copy form menu #item and menu mlid
        $form["menu"]["#item"] = $addform["menu"]["#item"];
        $form["menu"]["mlid"] = $addform["menu"]["mlid"];
        
      }
    }
    // modifify the delete object if we have a parent.
    if ($form['fnv_revises'] ) {
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
    }
    $form['buttonstop']['#weight'] = -100;
    $form['buttonstop']['#type'] = 'fieldset';
    $form['buttonstop']['#attributes'] = array('class'=>"full_node_version_status");
    $form['buttonstop']['submit'] = $form['buttons']['submit'] ;
    $form['buttonstop']['preview']   = $form['buttons']['preview'];
    $form['buttonstop']['delete']   = $form['buttons']['delete'];

    //add our submit function
    array_unshift($form["#validate"],"full_node_revision_node_validate");
    array_push($form["#submit"],"full_node_revision_node_submit");
  global $user;
  }
}
/*
 * Implementation of form validate
 * This function checks to ensure that the path is converted a draft path
 */
function full_node_revision_node_validate($form, &$form_state) {

  global $user;
  //Append Draft to end of path if we have a parent record and  there are no paths like it.
  if ($parent_nid = $form_state['values']['fnv_revises']) {
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
}
/** 
 * This function is the submit function for fnv_node_form
 * It purpose is to process changes to the parent node status
 */
function full_node_revision_node_submit($form, &$form_state) {
  $status_action = $form_state['values']['fnv_flow_status'];
  $status_action_p = $form_state['values']['fnv_flow_status_previous'];
  if ($status_action == $status_action_p) { $status_action = "none";}
  if ($form["nid"]["#value"] == NULL) {
      $form_state['values']['fnv_is_testing'] = 0;
      $form_state['values']['fnv_is_current'] = 0;
      $form_state['values']['fnv_is_draft']   = 0;
      $form_state['values']['fnv_parent_is_superseded']   = 1;
  }
  switch($status_action) {
    case "make_draft":
    // set parent to testing

      $form_state['values']['fnv_parent_is_testing']   = 1;
      $form_state['values']['fnv_is_draft']   = 1;
      $form_state['values']['fnv_is_testing'] = 0;
    break;
    case "make_testing" :
    // remove parent testing
      $form_state['values']['fnv_parent_is_testing']   = 0;
      $form_state['values']['fnv_is_testing'] = 1;
    break;
    case "make_live":
    // set parent to not current 
      //_draft_to_current($form["nid"]["#value"] , $parent_nid);


      $form_state['values']['fnv_is_testing'] = 1;
      $form_state['values']['fnv_is_current'] = 1;
      $form_state['values']['fnv_is_draft']   = 0;
      $form_state['values']['fnv_is_draft']   = 0;
      $form_state['values']['path'] = ereg_replace( "\_\(draft\)$","",$form_state['values']['path'] );
 
      //ajust parent node
      $form_state['values']['fnv_parent_is_testing']   = 0;
      $form_state['values']['fnv_parent_is_current']   = 0;
      global $user;
      $user->view_superseded = 1;
      if($parent_node = node_load($form_state['values']['fnv_revises'])) {
        $parent_node->path .= "-" . $parent_node->nid; 
        node_save($parent_node);
      }
    break;
  }
}
/*
 * This function change the status of the parent node before the child is removed
 * removing the superseded status and seting the testing status if the draft was 
 * testing
 */
function full_node_revision_node_revert($form, &$form_state) { 
  global $user;
  $user->view_superseded = 1;
  if($parent_node = node_load($form_state['values']['fnv_revises'])) {
    // parent is no longer superseded
    $parent_node->fnv_is_superseded = 0;
    // if child whas in testing put parent back in testing
    if ($form['#node']->fnv_is_testing) {
      $parent_node->fnv_is_testing = 1;
    }
    node_save($parent_node);
  }

}
