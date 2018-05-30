<?php

function game_service_resource_game_retrieve($game_id){
  global $base_url;
  $node = node_load($game_id);
  if(isset($node->nid)) {
    $response->nid = $node->nid;
    $response->title = $node->title;
    $response->created_on = date('jS F Y',$node->created);
    $response->description = ($node->body[LANGUAGE_NONE])?$node->body[LANGUAGE_NONE][0]['value']:"";
    $response->thumbnail = file_create_url($node->field_game_thumbnail[LANGUAGE_NONE][0]['uri']);
    $response->iframe = $node->field_is_iframe[LANGUAGE_NONE][0]['value'];
    $embed = '';
    if($response->iframe == 1){
      $embed = $node->field_game_iframe[LANGUAGE_NONE][0]['value'];
    } else {
      $embed = $node->field_game_link[LANGUAGE_NONE][0]['value'];
    }
    $response->embed = $embed;
    $get_game_query = db_select('node','n');
    $get_game_query->leftjoin('field_data_body','fdb','fdb.entity_id = n.nid');
    $get_game_query->leftjoin('field_data_field_game_thumbnail','fgt','fgt.entity_id = n.nid');
    $get_game_query->leftJoin('file_managed','fm','fm.fid = fgt.field_game_thumbnail_fid');
    $get_game_query->fields('n',array('title','nid','created'));
    $get_game_query->fields('fdb',array('body_value'));
    $get_game_query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'thumbnail');
    $get_game_query->condition('n.type','games');
    $get_game_query->condition('n.status',1);
    $get_game_query->condition('n.nid',$game_id,'<>');
    $get_game_query->groupBy('n.nid');
    $get_game_query->orderBy('n.created','DESC');
    $get_game_query->range(0,3);
    $other_game = $get_game_query->execute()->fetchAll();
    if(isset($other_game) && !empty($other_game)) {
      $response->other_games = $other_game;
    } else {
      $response->other_games = NULL;
    }

    return $response;
  }
  
  return false;
}

function game_service_resource_game_listing_retrieve() {
  global $base_url;
  
  $get_game_query_first = db_select('node','n');
  $get_game_query_first->leftjoin('field_data_body','fdb','fdb.entity_id = n.nid');
  $get_game_query_first->leftjoin('field_data_field_game_thumbnail','fgt','fgt.entity_id = n.nid');
  $get_game_query_first->leftJoin('file_managed','fm','fm.fid = fgt.field_game_thumbnail_fid');
  $get_game_query_first->fields('n',array('title','nid','created'));
  $get_game_query_first->addField('fdb','body_value','description');
  $get_game_query_first->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'thumbnail');
  $get_game_query_first->condition('n.type','games');
  $get_game_query_first->condition('n.status',1);
  $get_game_query_first->groupBy('n.nid');
  $get_game_query_first->orderBy('n.created','DESC');
  $get_game_query_first->range(0,2);
  $record_first = $get_game_query_first->execute()->fetchAll();

  if(count($record_first)>0) {
    $response['latest_game'] = $record_first;
    foreach($record_first as $exclude_game) {
      $game_id[] = $exclude_game->nid;
    }
   
    $get_game_query = db_select('node','n')->extend('PagerDefault');
    $get_game_query->leftjoin('field_data_body','fdb','fdb.entity_id = n.nid');
    $get_game_query->leftjoin('field_data_field_game_thumbnail','fgt','fgt.entity_id = n.nid');
    $get_game_query->leftJoin('file_managed','fm','fm.fid = fgt.field_game_thumbnail_fid');
    $get_game_query->fields('n',array('title','nid','created'));
    $get_game_query->addField('fdb','body_value','description');
    $get_game_query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'thumbnail');
    $get_game_query->limit(12);
    $get_game_query->condition('n.type','games');
    $get_game_query->condition('n.status',1);
    $get_game_query->condition('n.nid',$game_id,'NOT IN');
    $get_game_query->groupBy('n.nid');
    $get_game_query->orderBy('n.created','DESC');
    $record = $get_game_query->execute()->fetchAll();
    $response['all_games'] = $record;
    global $pager_total_items;
    $response['total_games'] = $pager_total_items[0];
    return $response;
  }
  
  return false;
}