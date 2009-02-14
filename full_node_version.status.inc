<?php


/*
 * implimentations of hook_nodeapi()
 */
function full_node_version_nodeapi(&$node, $op) {
  
  $type = $node->type;
  $full_node_version_node_types = variable_get('full_node_version_node_types',array());
  if (($op=='delete' )) {
    $query = db_query('DELETE FROM {full_node_version} WHERE nid = %d',$node->nid);

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
             WHERE c.nid =%d',$node->nid));
     $revised_by = db_result(db_query('SELECT * FROM {full_node_version} WHERE revises =%d',$node->nid));

     $node->fnv_is_draft      = $row->is_draft;
     $node->fnv_is_testing    = $row->is_testing;
     $node->fnv_is_current    = $row->is_current;
     $node->fnv_is_superseded = $row->is_superseded;
     $node->fnv_parent_is_draft      = $row->parent_is_draft;
     $node->fnv_parent_is_testing    = $row->parent_is_testing;
     $node->fnv_parent_is_current    = $row->parent_is_current;
     $node->fnv_parent_is_superseded = $row->parent_is_superseded;
     $node->fnv_revises       = $row->revises;
     $node->fnv_revised_by    = $revised_by;
  }
  if (($op=='view' ) && 
       in_array($type, $full_node_version_node_types)
     ) {
  }
  if(($op=='insert' || $op=='update') &&
       in_array($type, $full_node_version_node_types)
     ) {
    $query = db_query('
    INSERT INTO {full_node_version} (nid,is_draft,is_testing, is_current, is_superseded,revises)
    VALUES(%d, %d, %d,%d,%d,%d) 
    ON DUPLICATE KEY UPDATE 
      is_draft =%d, 
      is_testing =%d, 
      is_current =%d, 
      is_superseded =%d, 
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
    if ($node->fnv_revises) {
      $query = db_query('
      INSERT INTO {full_node_version} (nid,is_draft,is_testing, is_current, is_superseded)
      VALUES(%d, %d, %d,%d,%d) 
      ON DUPLICATE KEY UPDATE 
        is_draft =%d, 
        is_testing =%d, 
        is_current =%d, 
        is_superseded =%d',
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
      $parent_node=node_load($node->fnv_revises,NULL, TRUE);
      node_access_acquire_grants($parent_node);
    }
    if ($node->fnv_revised_by) {
      $query = db_query('
      INSERT INTO {full_node_version} (nid,revises)
      VALUES(%d, %d) 
      ON DUPLICATE KEY UPDATE 
      revises = %d',
        $node->fnv_revised_by,
        $node->nid,
        $node->nid
      );
    }
  }
}
/*
 * retrives node that match the status params
 * the status has an array of status for the child node and parent_status an array of status for the parent node
 * @PARAM status:
 *   an array whose key can be is_current, is_draft, is_testing, is_superseded and values are (1,0)
 * @PARAM parent_status:
 *   an array whose key can be is_current, is_draft, is_testing, is_superseded and values are (1,0)
 *   or it can be true or false
 * @return
 *  and array of node object created by node_load
 */
function full_node_version_get_nodes($status=array(), $has_parent=NULL) {
  $statuses = array('c.is_current', 'c.is_draft', 'c.is_testing','c.is_superseded','p.is_current', 'p.is_draft', 'p.is_testing','p.is_superseded');
  $c_statuses = array('is_current', 'is_draft', 'is_testing','is_superseded');
  $where_items=array();
  foreach($status as $k=>$v) {
    $operator = '=';
    $k = in_array($k, $c_statuses) ? "c.$k" : $k;
    if (in_array($k, $statuses)) {
      if(substr($v,0,1)=="!") {
        $operator = '!=';
        $v = substr($v,1);
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
  if ($has_parent === true)  { $where_items['p.nid is %s'] = 'NOT NULL';}
  if ($has_parent === false) { $where_items['p.nid is %s'] = 'NULL';}
  $where_clause = implode(' and ', array_keys($where_items));
  if ($where_clause) {$where_clause = "WHERE $where_clause";}
  $result = db_query(
      "SELECT c.nid 
         FROM {full_node_version} c 
    LEFT JOIN {full_node_version} p ON c.revises = p.nid
              $where_clause
    ",$where_items);
  $nodes = array();
  while($row = db_fetch_object($result)) {
   $nodes[$row->nid] = node_load($row->nid);
  }
  return $nodes;
}
$nodes = full_node_version_get_nodes($status=array('is_current'=>'!c.is_draft'));
foreach ($nodes as $n) {
}

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
  //$new_nid = db_result(db_query("select nid from {full_node_version} where revises = %d", $nid));
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
  $nodes = full_node_version_get_nodes(array('is_testing'=>'!is_current'));
  foreach ($nodes as $node) {
    if ($node->fnv_is_testing && $node->fnv_is_draft) {
      drupal_set_message('draft of '.$node->title ." published");
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

function full_node_version_set_status($nid, $status=NULL, $revises=NULL) {

  if(isset($status)) {
  $status_code += in_array('draft',$status)      ? 8 : 0;
  $status_code += in_array('current',$status)    ? 4 : 0;
  $status_code += in_array('testing',$status)    ? 2 : 0;
  $status_code += in_array('superseded',$status) ? 1 : 0;
  }

  if (isset($status) && isset($revises)) {
    $query = db_query('INSERT INTO {full_node_version} (nid,status,revises) VALUES(%d, %d, %d) ON DUPLICATE KEY UPDATE status=%d, revises = %d',$nid, $status_code, $revises, $status_code, $revises);
  }
  elseif (isset($revises)) {
    $query = db_query('INSERT INTO {full_node_version} (nid,status,revises) VALUES(%d, 0, %d) ON DUPLICATE KEY UPDATE revises = %d',$nid, $revises, $revises);
  }



}
/*
class full_node_version_status {
  private  $status =  array();

  function __construct($status_code) {
    if (($status_code & 8) == 8) {
      $this->status[] = 'draft';
    }
    if (($status_code & 4) == 4) {
      $this->status[] = 'current';
    }
    if (($status_code & 2) == 2) {
      $this->status[] = 'testing';
    }
    if (($status_code & 1) == 1) {
      $this->status[] = 'superseded';
    }
  }
  public function statusCode() {
    $status_code += in_array('draft',$this->status)      ? 8 : 0;
    $status_code += in_array('current',$this->status)    ? 4 : 0;
    $status_code += in_array('testing',$this->status)    ? 2 : 0;
    $status_code += in_array('superseded',$this->status) ? 1 : 0;
    return $status_code;
  }
 
  public function addStatus($status) {
      return $this->name;
  }
  public function removeStatus($status) {
      return $this->name;
  }
  public function hasStatus($status) {
    
    return in_array($status,$this->status);    
   }
}
*/


