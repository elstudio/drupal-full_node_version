<?php
// $Id: full_node_version.admin.inc,v 1.1 2009/03/10 18:25:31 elstudio Exp $
/**
 * This function builds the admin page
 */
function full_node_version_admin() {
    return drupal_get_form('full_node_version_admin_form');
}
/**
 * This funciton builds the form for the admin page
 */
function full_node_version_admin_form($form_state) {
  $types = node_get_types();
  $options = array();
  $selected_types = variable_get('full_node_version_node_types', array());
  foreach ($types as $key => $type) {
  $options[$key] = t($type->name);
  }
  $form['versioned_types'] = array(
    '#type'          => 'checkboxes',
    '#title'         => t('Enable Full Node Version On'),
    '#description'   => t("The following node types will have full node versioning enabled."),
    '#options'       => $options,
    '#default_value' => $selected_types,
  );
  $form['move_comments'] = array(
    '#type'          => 'checkbox',
    '#title'         => 'Move comments on Publish',
    '#description'   => t('When enabled, when a draft is published, all of the current node comments will be move to the draft node.'),
    '#default_value' => variable_get("full_node_version_move_comments", FALSE),
  );

  $form['use_cck_fields'] = array(
    '#type'          => 'checkbox',
    '#title'         => 'Put status and revises in CCK fields',
    '#description'   => t('When enabled, CCK fields Full Node Version Status and Revises will be populated with status on load (Requires Node Reference).'),
    '#default_value' => variable_get("full_node_version_use_cck_fields", FALSE),
  );
  if (!(module_exists("nodereference"))) {
  $form['use_cck_fields']['#attributes'] = array('disabled' => 'disabled');
   
  }
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}
/**
 * This is the Submit function for the admin form
 * for each selected type save to the database and create cck field on they type.
 */
function full_node_version_admin_form_submit($form, $form_state) {

  $selected_types = $form_state['values']['versioned_types'];
  $move_comments  = $form_state['values']['move_comments'] && TRUE;
  $use_cck_fields     = $form_state['values']['use_cck_fields'] && TRUE;
  $versioned_types = array();
  foreach ($selected_types as $type) {
    if ($type) {
      array_push($versioned_types, $type);
      if ($use_cck_fields) {
        _full_node_version_create_cck_fields($type) ;
      }
    }
  }
  
  if ($use_cck_fields) {
    if ((module_exists("nodereference"))) {
      _update_field_fnv_revised_types($versioned_types);
    }
  }
  else {
    _full_node_version_remove_cck_fields();
  }
  variable_set('full_node_version_node_types', $versioned_types);
  variable_set('full_node_version_move_comments', $move_comments);
  variable_set('full_node_version_use_cck_fields', $use_cck_fields);
  drupal_set_message(t('full Node Version Settings have been updated'));
  node_access_needs_rebuild(TRUE);
}

/*
 * This is a helper funciton for update the cck field field_fnv_revises that
 * allows all of the node types to be revised.
 * @param $versioned_types
 *     an array of content types that are in versioning
 */
function _update_field_fnv_revised_types($versioned_types) {
  foreach ($versioned_types as $type) {
    $new_types[$type] = $type;
  }
  $global_settings_string = db_result(db_query("select global_settings from {content_node_field} where field_name ='field_fnv_revises'"));
  $global_settings = unserialize($global_settings_string);

  $global_settings["referenceable_types"] = $new_types;
  $global_settings_string = serialize($global_settings);
  db_query("update {content_node_field} set global_settings = '%s' where field_name = 'field_fnv_revises'", $global_settings_string);
}


function _full_node_version_remove_cck_fields() {
   if ((module_exists("content"))) {
    module_load_include('inc', 'content', 'includes/content.crud');
    foreach (array_keys(node_get_types()) as $type) {
      content_field_instance_delete('field_fnv_revises', $type);
      content_field_instance_delete('field_fnv_status', $type);
    }
  }
}
/*
 * Creates fnv_revises and fvn_status cck on a content type
 *
 * This function checks to see of there is a fnv_revises and fnv_status cck on the 
 * node type if there is not then create it.
 *
 * @param $type
 *    A content type name
 */
function _full_node_version_create_cck_fields($type) {
  module_load_include('inc', 'content', 'includes/content.crud');
 

  $types = node_get_types();
  foreach ($types as $typename => $trash) {
    $types[$typename] = 0;
  }
  $types[$type] = $type;
  $field_fnv_revises = 
    array( 'field_name' => 'field_fnv_revises', 'type_name'
      => 'page', 'display_settings' => array( 'label' => array( 'format' =>
      'above', ), 'teaser' => array( 'format' => 'default', ), 'full' => array(
      'format' => 'default', ), 4 => array( 'format' => 'default', ), 2 => array(
      'format' => 'default', ), 3 => array( 'format' => 'default', ), ),
      'widget_active' => '1', 'type' => 'nodereference', 'required' => '0',
      'multiple' => '0', 'db_storage' => '1', 'module' => 'nodereference', 'active'
      => '1', 'locked' => '0', 'columns' => array( 'nid' => array( 'type' =>
      'int', 'unsigned' => TRUE, 'not null' => FALSE, ), ), 'referenceable_types' =>
      $types, 'advanced_view' => '--', 'advanced_view_args' => '',
      'widget' => array( 'default_value' => array( 0 => array( 'nid' => NULL,
      '_error_element' =>
      'default_value_widget][field_fnv_revises][0][nid][nid', ), ),
      'default_value_php' => NULL, 'label' => 'Revises', 'weight' => NULL,
      'description' => '', 'type' => 'nodereference_autocomplete', 'module' =>
      'nodereference', ), );

  $field_fnv_status = 
    array( 'field_name' => 'field_fnv_status', 'type_name' => 'page',
      'display_settings' => array( 'label' => array( 'format' => 'above', ),
      'teaser' => array( 'format' => 'default', ), 'full' => array( 'format' =>
      'default', ), 4 => array( 'format' => 'default', ), 2 => array( 'format' =>
      'default', ), 3 => array( 'format' => 'default', ), ), 'widget_active' => '1',
      'type' => 'text', 'required' => '0', 'multiple' => '3', 'db_storage' => '0',
      'module' => 'text', 'active' => '1', 'locked' => '0', 'columns' => array(
      'value' => array( 'type' => 'text', 'size' => 'big', 'not null' => FALSE,
      'sortable' => TRUE, ), ), 'text_processing' => '0', 'max_length' => '',
      'allowed_values' => "draft\ncurrent\nsuperseded\ntesting", 'allowed_values_php'
      => '', 'widget' => array( 'default_value' => array( 0 => array( 'value' =>
      '', '_error_element' => 'default_value_widget][field_fnv_status][value', ), ),
      'default_value_php' => NULL, 'label' => 'Full Node Version Status', 'weight' =>
      NULL, 'description' => '', 'type' => 'optionwidgets_buttons', 'module' =>
      'optionwidgets', ), );

  if (!(content_fields('field_fnv_status', $type))) {
    $field_fnv_status['type_name'] = $type;
    content_field_instance_create($field_fnv_status); 
  }
  if (!(content_fields('field_fnv_revises', $type))) {
    $field_fnv_revises['type_name'] = $type;
    content_field_instance_create($field_fnv_revises); 
  }
}
 
