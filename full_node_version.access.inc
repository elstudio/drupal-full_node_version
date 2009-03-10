<?php
// $Id: full_node_version.access.inc,v 1.1 2009/03/10 18:25:31 elstudio Exp $ 
/**
 * Implementation of  hook_node_access_records
 *
 * Creates  realms for testing, current, superseded, and working we all so
 * add a realm for all if the node is not a node type accosiated with full node version
 * which we are not doing currently.
 *
 */
function full_node_version_node_access_records($node) {
  $type = $node->type;
  $fnv_types =  variable_get('full_node_version_node_types', array());
  if ( in_array($type, $fnv_types) ) {
    if ($node->fnv_is_testing) {
      $grants[] = array(
        'realm' => 'fnv_testing',
        'gid' => 1,
        'grant_view' => TRUE,
        'grant_update' => FALSE,
        'grant_delete' => FALSE,
        'priority' => 0,
      );
    }
    if ($node->fnv_is_current) {
      $grants[] = array(
        'realm' => 'fnv_current',
        'gid' => 1,
        'grant_view' => TRUE,
        'grant_update' => FALSE,
        'grant_delete' => FALSE,
        'priority' => 0,
      );
    }
    if ($node->fnv_is_superseded) {
      $grants[] = array(
        'realm' => 'fnv_superseded',
        'gid' => 1,
        'grant_view' => TRUE,
        'grant_update' => FALSE,
        'grant_delete' => FALSE,
        'priority' => 0,
      );
    }
    else {
      $grants[] = array(
        'realm' => 'fnv_working',
        'gid' => 1,
        'grant_view' => TRUE,
        'grant_update' => TRUE,
        'grant_delete' => TRUE,
        'priority' => 0,
      );
    }
  }
  return $grants;
}

/*
 * Implementation of hook_grants
 *
 *  Sets grants based on the user permission for current, 
 *  testing, working and superseded.  
 *  It also grants all to gid 1 so that all content not associated with full node revision is displayed.
 */
function full_node_version_node_grants($account, $op) {
  global $user;
  if ( user_access('full node version test') 
    || $_SESSION['full_node_version_testing']
  ) {
    $grants['fnv_testing'] = array(1); 
    return $grants;
  }
  if ( user_access('full node version live')  
    || $user->uid == 0) {
    $grants['fnv_current'] = array(1); 
  }
  if ( user_access('full node version active') ) {
    $grants['fnv_working'] = array(1); 
  }
  if ($account->view_superseded) {
    $grants['fnv_superseded'] = array(1); 
  }
  $grants["all"] = array(0);

  return $grants;
  

}


