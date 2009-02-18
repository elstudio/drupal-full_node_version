<?php
/**
 * Implementation of hoo_translated_menu_link_alter
 *
 * used to change the title of the toggle testing linl
 */
function full_node_version_translated_menu_link_alter(&$item, $map) { 
  if ($item['link_path'] == 'admin/content/toggle-testing') {
    if ($_SESSION['full_node_version_testing'] == true) {
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
  if($_SESSION['full_node_version_testing'] == true) {
    $_SESSION['full_node_version_testing'] = false;
    drupal_set_message("Exited testing");
  }
  else {
    $_SESSION['full_node_version_testing'] = true;
    drupal_set_message("Entered testing");
  }
  drupal_goto();
}
/*
 * Implementation of hook_action_info()
 */
function full_node_version_action_info () {
  return array(
    'full_node_version_publish_testing' => array(
       'type' => 'system',
       'description' => t('Promote Testing to Published'),
       'configurable' => FALSE,
       )
  );
}

