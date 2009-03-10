<?php
// $Id: full_node_version.status.inc,v 1.1 2009/03/10 18:25:31 elstudio Exp $
/*
 * implimentations of hook_nodeapi()
 */
function full_node_version_nodeapi(&$node, $op) {
  
  $type = $node->type;
  $full_node_version_node_types = variable_get('full_node_version_node_types', array());
  if (($op=='delete' )) {
    $query = db_query('DELETE FROM {full_node_version} WHERE nid = %d', $node->nid);
  }
  if (($op=='load' ) && 
       in_array($type, $full_node_version_node_types)
     ) {
     $row = db_fetch_object(db_query(
       'SELECT c.*,
          p.is_draft as parent_is_draft,
          p.is_current as parent_is_current,
          p.is_testing as parent_is_testing,
          p.is_superseded as parent_is_superseded
          FROM {full_node_version} c 
          LEFT JOIN {full_node_version p ON p.nid = c.revises
          WHERE c.nid =%d', $node->nid
        ));
     $revised_by = db_result(db_query('SELECT * FROM {full_node_version} WHERE revises =%d', $node->nid));

     $node->fnv_is_draft              = $row->is_draft;
     $node->fnv_is_testing            = $row->is_testing;
     $node->fnv_is_current            = $row->is_current;
     $node->fnv_is_superseded         = $row->is_superseded;
     $node->fnv_parent_is_draft       = $row->parent_is_draft;
     $node->fnv_parent_is_testing     = $row->parent_is_testing;
     $node->fnv_parent_is_current     = $row->parent_is_current;
     $node->fnv_parent_is_superseded  = $row->parent_is_superseded;
     $node->fnv_revises               = $row->revises;
     $node->fnv_revised_by            = $revised_by;
     if ($node->field_fnv_status) {
       if ($node->fnv_is_draft) $field_fnv_status[]      = array("value" => "draft");
       if ($node->fnv_is_testing) $field_fnv_status[]    = array("value" => "testing");
       if ($node->fnv_is_current) $field_fnv_status[]    = array("value" => "current");
       if ($node->fnv_is_superseded) $field_fnv_status[] = array("value" => "superseded");
       $node->field_fnv_status=$field_fnv_status;
     }
     if ($node->field_fnv_revises) {
       $node->field_fnv_revises[0]['nid'] = $node->fnv_revises;
     }
  }
  if (($op=='view' ) && 
       in_array($type, $full_node_version_node_types)
     ) {
  }
  if (($op=='insert' || $op=='update') &&
       in_array($type, $full_node_version_node_types)
     ) {
    $query = db_query('
    INSERT INTO {full_node_version} (nid, is_draft, is_testing, is_current, is_superseded, revises)
    VALUES(%d, %d, %d, %d, %d, %d) 
    ON DUPLICATE KEY UPDATE 
      is_draft = %d, 
      is_testing = %d, 
      is_current = %d, 
      is_superseded = %d, 
      revises = %d',
      $node->nid,
      $node->fnv_is_draft,
      $node->fnv_is_testing,
      $node->fnv_is_current,
      $node->fnv_is_superseded,
      $node->fnv_revises,
      $node->fnv_is_draft,
      $node->fnv_is_testing,
      $node->fnv_is_current,
      $node->fnv_is_superseded,
      $node->fnv_revises
    );
    if ($node->fnv_revised_by) {
      $query = db_query('
      INSERT INTO {full_node_version} (nid, revises)
      VALUES(%d, %d) 
      ON DUPLICATE KEY UPDATE 
      revises = %d',
        $node->fnv_revised_by,
        $node->nid,
        $node->nid
      );
    }
  }
  if (($op=='insert' || $op=='update' || $op=='deleted') &&
       in_array($type, $full_node_version_node_types)
     ) {
    if ($node->fnv_revises) {
      $query = db_query('
      INSERT INTO {full_node_version} (nid, is_draft, is_testing, is_current, is_superseded)
      VALUES(%d, %d, %d, %d, %d) 
      ON DUPLICATE KEY UPDATE 
        is_draft = %d, 
        is_testing = %d, 
        is_current = %d, 
        is_superseded = %d',
        $node->fnv_revises,
        $node->fnv_parent_is_draft,
        $node->fnv_parent_is_testing,
        $node->fnv_parent_is_current,
        $node->fnv_parent_is_superseded,
        $node->fnv_parent_is_draft,
        $node->fnv_parent_is_testing,
        $node->fnv_parent_is_current,
        $node->fnv_parent_is_superseded
      );
      $parent_node=node_load($node->fnv_revises, NULL, TRUE);
      node_access_acquire_grants($parent_node);
    }
  }
}
/*
 * retrives node that match the status params
 * the status has an array of status for the child node and parent_status an array of status for the parent node
 * @PARAM status:
 *   an array whose key can be is_current, is_draft, is_testing, is_superseded and values are (1, 0)
 * @PARAM parent_status:
 *   an array whose key can be is_current, is_draft, is_testing, is_superseded and values are (1, 0)
 *   or it can be TRUE or FALSE
 * @return
 *  and array of node object created by node_load
 */
function full_node_version_get_nodes($status=array(), $has_parent=NULL) {
  $statuses = array('c.is_current', 'c.is_draft', 'c.is_testing', 'c.is_superseded', 'p.is_current', 'p.is_draft', 'p.is_testing', 'p.is_superseded');
  $c_statuses = array('is_current', 'is_draft', 'is_testing', 'is_superseded');
  $where_items=array();
  foreach ($status as $k => $v) {
    $operator = '=';
    $k = in_array($k, $c_statuses) ? "c.$k" : $k;
    if (in_array($k, $statuses)) {
      if (substr($v, 0, 1)=="!") {
        $operator = '!=';
        $v = substr($v, 1);
      }
      $v = in_array($v, $c_statuses) ? "c.$v" : $v;
      if (in_array($v, $statuses)) {
        $where_items["$k $operator $v"]    = NULL;
      }
      else {
        $where_items["$k $operator %d"]    = $v;
      }
    }
  }
  if ($has_parent === TRUE)  { 
    $where_items['p.nid is %s'] = 'NOT NULL';
  }
  if ($has_parent === FALSE) { 
    $where_items['p.nid is %s'] = 'NULL';
  }
  $where_clause = implode(' and ', array_keys($where_items));
  if ($where_clause) {
    $where_clause = "WHERE $where_clause";
  }
  $result = db_query(
      "SELECT c.nid 
         FROM {full_node_version} c 
    LEFT JOIN {full_node_version} p ON c.revises = p.nid
              $where_clause
    ", $where_items);
  $nodes = array();
  while ($row = db_fetch_object($result)) {
   $nodes[$row->nid] = node_load($row->nid);
  }
  return $nodes;
}

/* 
 * Takes testing status and moves it to current status
 *
 * IF not testing and current then remove current
 * IF testing and draft then run draft_to_current
 * IF testing and not draft  then add current
 * @param $nids 
 *   an array of node nids to the function on
 */
function full_node_version_publish_testing($nids = TRUE) {
  $nodes = full_node_version_get_nodes(array('is_testing' => '!is_current'));
  foreach ($nodes as $node) {
    if ($node->fnv_is_testing && $node->fnv_is_draft) {
      drupal_set_message('draft of '. $node->title ." published");
    }
    elseif ($node->fnv_is_testing) {
      drupal_set_message($node->title ." published");
    }
    else {
      drupal_set_message($node->title ." unpublished");
    }
    $node->fnv_is_current = $node->fnv_is_testing;
    $node->fnv_is_draft = 0;
    node_save($node);
  }
}


function full_node_version_fnv_publish_draft($draft_nid, $current_nid) {
  // =======  For Panels ==========//
  if (module_exists("panels")) {
    $panes = array();
    $result = db_query("SELECT * FROM {panels_pane} WHERE type = 'node'");
    while ($candidate = db_fetch_object($result)) {
      $candidate->configuration = unserialize($candidate->configuration);
      if ($current_nid == $candidate->configuration['nid']) {
        array_push($panes, $candidate);
      }
    }
    foreach ($panes as $pane) {
      $pane->configuration['nid'] = $draft_nid;
      $query = "UPDATE {panels_pane} SET configuration = '%s' where pid = %d";
      db_query($query, serialize($pane->configuration), $pane->pid);
      drupal_set_message("Pane Updated");
    }
  }

}
function _full_node_version_new_draft_panel($child_did, $parent_did) {


db_query("UPDATE {panels_display} child , {panels_display} parent 
  SET child.layout_settings = parent.layout_settings,
    child.panel_settings = parent.panel_settings,
    child.layout = parent.layout 
  WHERE child.did = %d 
  AND parent.did = %d",
  $child_did, $parent_did);
db_query("INSERT INTO {panels_pane} (did, panel, type, subtype, shown, access, visibility, configuration, cache, position) SELECT %d, panel, type, subtype, shown, access, visibility, configuration, cache, position FROM {panels_pane} WHERE did = %d", $child_did, $parent_did);


}
function full_node_version_file_references($file) {
  $field_name = $file->field_name;
  var_dump($file);
  $fid = $file->fid;

  if ( db_table_exists("content_$field_name") ) {
    $query = "SELECT count(nid) FROM {content_%s} where %s_fid =%d";
    $count = db_result(db_query($query, $field_name, $field_name, $fid));
  }
  else {
  $query ="SELECT type_name FROM {content_node_field_instance} WHERE field_name = '%s'";
  $table_name = db_result(db_query($query, $field_name));
    $query = "SELECT count(nid) FROM {content_type_%s} where %s_fid =%d";
    $count = db_result(db_query($query, $table_name, $field_name, $fid));
  }
  if ($count > 1) { 
    return TRUE; 
  }
  else { return FALSE;}
}

