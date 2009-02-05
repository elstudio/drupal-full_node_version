<?php
/*
 * Adds a status to a node
 * 
 * @param $n
 *   a nid or node object
 * @param $status_in
 *   the status to add
 *
 */
function full_node_version_status_add($n, $status) {
  if ($n ==NULL) {return;}
  $node_in = $n;
  if (!is_object($n)) {
    $node_in = node_load($n, NULL, TRUE);
  }
  if (is_object($node_in)) {
    if (!$node_in->field_fnv_status) {
      $node_in->field_fnv_status = array();
    }
    $status_array = array_map(create_function('$a',' return $a["value"];'),$node_in->field_fnv_status);
    if (!in_array($status,$status_array) ){
      array_push( $node_in->field_fnv_status , array("value"=>$status) );
    }
    node_save($node_in);
  }
}

/*
 * Removes a status from a node
 * 
 * @param $n
 *   a nid or node object
 * @param $status_in
 *   the status to remove 
 *
 */
function full_node_version_status_remove($n, $status_in) {
  if ($n == NULL) {return;}
  $node_in = $n;
  if (!is_object($n)) {
    // TODO: This was cached, incorrectly?
    $node_in = node_load($n, NULL, true);
  }
  DEBUG && drupal_set_message("full_node_version_status_remove($n->nid, $status_in)");
  if (is_object($node_in) && is_array($node_in->field_fnv_status)) {
    $status_array = array_map(create_function('$a',' return $a["value"];'),$node_in->field_fnv_status);
    if ( in_array( $status_in , $status_array ) ) {
      $key = array_search($status_in,$status_array);
      array_splice( $node_in->field_fnv_status , $key,1 );
    }
    node_save($node_in);
  }
}
/**
 * Returns if node has a particular status
 *
 * @param $n
 *   a nid or node object
 * @param $status_in
 *   the status to remove 
 * @return
 *   a bool, true if the node has the particular status
 *
 */
function full_node_version_is_status($n, $status_in) {

  if ($n == NULL) { return false;}
  $node_in = $n;
  if (!is_object($n)) {
    $node_in = node_load($n, NULL, true);
  }
  if (is_object($node_in) && is_array($node_in->field_fnv_status)) {
    $status_array = array_map(create_function('$a' ,'return $a["value"];'), $node_in->field_fnv_status);
    if ( in_array( $status_in , $status_array ) ) {
      // DEBUG && drupal_set_message("full_node_version_is_status($n->nid, $status_in) == TRUE");
      return true;
    }
    else {
      // DEBUG && drupal_set_message("full_node_version_is_status($n->nid, $status_in) == false");
      return false;
    }
  }
}
/**
 * Returns node that is revised by the input node
 *
 * @param $n
 *   a nid or node object
 * @return
 *   a node object
 *
 */
function full_node_version_revised_by($n) {
  if ($n == NULL) { return false;}
  $nid = $n;
  if (is_object($n)) {
    $nid = $n->nid;
  }
  $new_nid = db_result(db_query("select nid from {content_field_fnv_revises} where field_fnv_revises_nid = %d", $nid));
  return node_load($new_nid, NULL, true);


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
function full_node_version_publish_testing($nids = true) {
  $nodes = _full_node_version_status_of_nodes_array();
  foreach ($nodes as $nid => $node_item) {
    if (in_array($nid, $nids) || $nids == $true)  {
      $n = node_load($nid);
      if ($node_item['t'] == 0) {
        if ($node_item['c'] == 1) {
          full_node_version_status_remove($nid, "current");
          drupal_set_message($n->title ." unpublished");
        }
        elseif ($node_item['d'] == 1) {
          $current_nid = $n->field_fnv_revises[0]["nid"];
          if (($parent_s =$nodes[$current_nid]) && ($parent_s['t'] != $parent_s['c'])) {
            if($parent_s['t'] == 1) {
              full_node_version_status_add($current_nid, "current");
              drupal_set_message($n->title ." published");
            }
            else {
              full_node_version_status_remove($current_nid, "current");
              drupal_set_message($n->title ." unpublished");
            }
            
          }

        }
      }  
      else {
        if ($node_item['d'] == 1) {
          $current_nid = $n->field_fnv_revises[0]["nid"];
          _draft_to_current($nid, $current_nid);
          drupal_set_message("Draft of ".$n->title." published");
        }
        else {
          full_node_version_status_add($nid, "current");
          drupal_set_message($n->title." published");
        }
      }
    }
  }

}


/*
 * This is a helper function for publish a draft to current status
 *
 */
function _draft_to_current($draft_nid = NULL , $current_nid = NULL ) {

  if ($current_nid) {
    drupal_set_message("Unpublishing current node $current_nid.");
    $current_node = node_load($current_nid, NULL, true);
    full_node_version_status_remove($current_node, "testing");
    $current_node = node_load($current_nid, NULL, true);
    full_node_version_status_remove($current_node, "current");
    path_set_alias("node/$current_nid", NULL, NULL, 'en');
    path_set_alias("node/$current_nid", $current_node->path ."-". $current_node->nid, NULL, 'en');
  }
  if ($draft_nid) {
    drupal_set_message("Publishing draft node $draft_nid.");
    $draft_node = node_load($draft_nid, NULL, true);
    full_node_version_status_add($draft_node, "testing");
    $draft_node = node_load($draft_nid, NULL, true);
    full_node_version_status_add($draft_node, "current");
    $draft_node = node_load($draft_nid, NULL, true);
    full_node_version_status_remove($draft_node, "draft");
    $draft_node = node_load($draft_nid, NULL, true);
    if (module_exists("path")) {
      path_set_alias("node/$draft_nid", NULL, NULL, 'en');
      path_set_alias("node/$draft_nid", ereg_replace  ( "\_\(draft\)$"  ,"" , $draft_node->path), NULL, 'en');
    }
    if (variable_get("full_node_version_move_comments", false) ) {
      db_query("UPDATE {comments} SET nid = %d WHERE nid = %d", $draft_nid, $current_nid);
    }
  }
  DEBUG && drupal_set_message("module_invoke_all(\"fnv_publish_draft\", $draft_nid, $current_nid) DISABLED");
  // FIXME: Disabled for debugging purposes
  // module_invoke_all("fnv_publish_draft", $draft_nid, $current_nid);
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
    foreach($panes as $pane) {
      $pane->configuration['nid'] = $draft_nid;
      $query = "UPDATE {panels_pane} SET configuration = '%s' where pid = %d";
      db_query($query, serialize($pane->configuration), $pane->pid);
      drupal_set_message("Pane Updated");
    }
  }

}

/* 
 * returns an array of node status of the shape
 * $return[nid][d] = 0/1
 *             [c] = 0/1
 *             [t] = 0/1
 *             [s] = 0/1
 */
function _full_node_version_status_of_nodes_array() {
  
  $fnv_types = variable_get('full_node_version_node_types',array());
  $types_string = "";
  foreach ($fnv_types as $type) {
    $types_string .= "'$type',";
  }
  if ($types_string) {
    $types_string = ereg_replace (",$","",$types_string);
    $query = "
      SELECT 
        n.nid, 
        CASE WHEN d.nid IS NOT NULL THEN 1 ELSE 0 END as d, 
        CASE WHEN c.nid IS NOT NULL THEN 1 ELSE 0 END as c, 
        CASE WHEN t.nid IS NOT NULL THEN 1 ELSE 0 END as t, 
        CASE WHEN s.nid IS NOT NULL THEN 1 ELSE 0 END as s
      FROM node n
      LEFT JOIN content_field_fnv_status d ON n.nid = d.nid AND n.vid=d.vid AND d.field_fnv_status_value = 'draft'
      LEFT JOIN content_field_fnv_status c ON n.nid = c.nid AND n.vid=c.vid AND c.field_fnv_status_value = 'current'
      LEFT JOIN content_field_fnv_status t ON n.nid = t.nid AND n.vid=t.vid AND t.field_fnv_status_value = 'testing'
      LEFT JOIN content_field_fnv_status s ON n.nid = s.nid AND n.vid=s.vid AND s.field_fnv_status_value = 'superseded'
      WHERE type IN ($types_string)
      ";
    $result = db_query($query);
    while ($row = db_fetch_array($result)) {
      $return[$row['nid']] = $row;
    }
    return $return;
  }
}
/*
 * This is a helper funciton for update the cck field field_fnv_revises that
 * allows all of the node types to be revised.
 */
function _update_field_fnv_revised_types($versioned_types) {
  foreach ($versioned_types as $type) {
    $new_types[$type] = $type;
  }
  $global_settings_string = db_result(db_query("select global_settings from {content_node_field} where field_name ='field_fnv_revises'"));
  $global_settings = unserialize($global_settings_string);

  $global_settings["referenceable_types"] = $new_types;
  $global_settings_string = serialize($global_settings);
  db_query("update {content_node_field} set global_settings = '%s' where field_name = 'field_fnv_revises'",$global_settings_string);


}
function _full_node_version_new_draft_panel($child_did, $parent_did) {


db_query("UPDATE {panels_display} child , {panels_display} parent SET child.layout_settings=parent.layout_settings,child.panel_settings=parent.panel_settings,child.layout=parent.layout WHERE child.did=%d and parent.did=%d",$child_did, $parent_did);
db_query("INSERT INTO panels_pane (did, panel, type, subtype, shown, access, visibility, configuration, cache, position) SELECT %d, panel, type, subtype, shown, access, visibility, configuration, cache, position FROM panels_pane WHERE did = %d", $child_did, $parent_did);


}
function full_node_version_file_references($file) {
  $field_name = $file->field_name;
  var_dump($file);
  $fid = $file->fid;

  if ( db_table_exists("content_$field_name") ) {
    $query = "SELECT count(nid) FROM {content_%s} where %s_fid =%d";
    $count = db_result(db_query($query,$field_name,$field_name, $fid));
  }
  else {
  $query ="SELECT type_name FROM content_node_field_instance WHERE field_name = '%s'";
  $table_name = db_result(db_query($query,$field_name));
    $query = "SELECT count(nid) FROM {content_type_%s} where %s_fid =%d";
    $count = db_result(db_query($query,$table_name,$field_name, $fid));
  }
  if ($count > 1) { return true; }
  else { return false;}
}
