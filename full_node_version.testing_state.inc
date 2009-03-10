<?php
// $Id: full_node_version.testing_state.inc,v 1.1 2009/03/10 18:25:31 elstudio Exp $
/**
 * Implementation of hoo_translated_menu_link_alter
 *
 * used to change the title of the toggle testing linl
 */
function full_node_version_translated_menu_link_alter(&$item, $map) { 
  if ($item['link_path'] == 'admin/content/toggle-testing') {
    if ($_SESSION['full_node_version_testing'] == TRUE) {
      $item['title'] = 'Exit Testing';
    }
    else {
      $item['title'] = 'Enter Testing';
    }
    if (isset($item['localized_options']['query'])) {
      if ($item['localized_options']['query'] == 'destination') {                          
        $item['localized_options']['query'] = drupal_get_destination();                    
      } 
    }  
  }
}
/**
 * is the toggle page for enter and exiting testing
 */
function full_node_version_toggle_testing() {
  if ($_SESSION['full_node_version_testing'] == TRUE) {
    $_SESSION['full_node_version_testing'] = FALSE;
    drupal_set_message("Exited testing");
  }
  else {
    $_SESSION['full_node_version_testing'] = TRUE;
    drupal_set_message("Entered testing");
  }
  drupal_goto();
}
/*
 * Implementation of hook_action_info()
 */
function full_node_version_action_info() {
  return array(
    'full_node_version_publish_testing' => array(
       'type' => 'system',
       'description' => t('Promote Testing to Published'),
       'configurable' => FALSE,
       )
  );
}

