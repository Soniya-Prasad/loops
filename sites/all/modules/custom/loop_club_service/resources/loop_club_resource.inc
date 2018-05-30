<?php
function loop_club_service_resource_create_group($data) {
  if(isset($data) && !empty($data)) {
    global $user;
    $title = filter_xss($data['title']);
    $description = filter_xss($data['group_description']);
    $image_fid = filter_xss($data['group_image']);
    $group_participant = filter_xss($data['group_participant']);
    $group_admin = filter_xss($data['group_admin']);
    $group_new_participant = filter_xss($data['new_participant']);
    if(!isset($title) || empty($title)){
      return services_error(t('Group Title cannot be null or empty.'), 406);
    }
    
    if(isset($data['nid']) && !empty($data['nid'])) {
      $node = node_load($data['nid']); 
    } else {
      $node = new stdClass();
      $node->type = "loop_club";
      node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
      $node->language = LANGUAGE_NONE; // Or e.g. 'en' if locale is enabled
      $node->uid = $user->uid; 
      $node->status = 1; //(1 or 0): published or not
      $node->promote = 0; //(1 or 0): promoted to front page
      $node->comment = 0; // 0 = comments disabled, 1 = read only, 2 = read/write
    
    }
    if(isset($title) && !empty($title)){
      $node->title = $title;
    }
    
    // Description 
    if(isset($description) && !empty($description)) {
      $node->body[LANGUAGE_NONE][0]['value'] = $description;
    }
    
    // Group Image
    if(isset($image_fid) && !empty($image_fid)) {
      $node->field_group_image[LANGUAGE_NONE][0]['fid'] = $image_fid;
    }
  
    // Group Participant - Entity reference field
    if(isset($group_participant) && count($group_participant)>0) {
      $participants = '';
      unset($node->field_participant[LANGUAGE_NONE]);
      $params['group_name'] = $title;
      foreach($group_participant as $key => $participant) {
        $participants.= $participant.',';
        if(!isset($data['nid']) || empty($data['nid'])) {
          drupal_mail('loop_club_service', 'loop_club_add',$participant,$language=array(),$params,NULL);
        }
      }
      $node->field_participant[LANGUAGE_NONE][0]['value'] = substr($participants,0,-1);
    }
    else {
      unset($node->field_participant);
    }

    if(isset($data['nid']) && !empty($data['nid'])) {
      foreach($group_new_participant as $key => $new_participant) {
        drupal_mail('loop_club_service', 'loop_club_add',$new_participant,$language=array(),$params,NULL);
      }
    }
    if(isset($group_admin) && !empty($group_admin)){
      $admins = '';
      unset($node->field_group_admin[LANGUAGE_NONE]);
      foreach($group_admin as $key => $admin) {
        $admins.= $admin.',';
      }
      $node->field_group_admin[LANGUAGE_NONE][0]['value'] = substr($admins,0,-1);
    } else {
      $node->field_group_admin[LANGUAGE_NONE][0]['value'] = $user->mail;
    }
    $node->field_private_group[LANGUAGE_NONE][0]['value'] = 1;
    $node->field_group_closed[LANGUAGE_NONE][0]['value'] = 0;

    $node = node_submit($node); // Prepare node for saving
    node_save($node);
    
    $node = node_load($node->nid);
    $response = new stdClass();
    $account = user_load($node->uid);
    $response->nid = $node->nid;
    $response->title = $node->title;
    $response->created_by = ($account->field_full_name[LANGUAGE_NONE])?$account->field_full_name[LANGUAGE_NONE][0]['value']:NULL;
    $response->created_on = date('jS F Y',$node->created);
    $response->description = $node->body[LANGUAGE_NONE][0]['value'];
    $response->group_closed = isset($node->field_group_closed)?$node->field_group_closed[LANGUAGE_NONE][0]['value']:0;
    $response->group_image = file_create_url($node->field_group_image[LANGUAGE_NONE][0]['uri']);
    
    
    if(isset($node->field_participant[LANGUAGE_NONE]) && !empty($node->field_participant[LANGUAGE_NONE])){
      $participants_raw = $node->field_participant[LANGUAGE_NONE][0]['value'];
      $participants = explode(',',$participants_raw);
      foreach($participants as $participant) {
        $response->participants[] = array('email' => $participant,'role'=>'participant');
      }
    }
    if(isset($node->field_group_admin[LANGUAGE_NONE]) && !empty($node->field_group_admin[LANGUAGE_NONE])){
      $admins_raw = $node->field_group_admin[LANGUAGE_NONE][0]['value'];
      $admins = explode(',',$admins_raw);
      foreach($admins as $admin) {
        $response->participants[] = array('email' => $admin,'role'=>($account->mail == $admin)?'creator':'admin');
      }
    }
    return $response;
  } else {
    return services_error(t('Data cannot be empty'), 406);
  }
}

function loop_club_service_resource_set_group_admin($data) {
  if(isset($data) && !empty($data)) {
    $group_nid = filter_xss($data['nid']);
    $group_participant = filter_xss($data['group_participant']);
    $group_admin = filter_xss($data['group_admin']);
    $group_new_participant = filter_xss($data['new_participant']);
    if(!isset($group_nid) || empty($group_nid)) {
      return services_error(t('Group id cannot be null or empty.'), 406);
    }
    if(!isset($group_admin) || empty($group_admin)) {
      return services_error(t('Group admin cannot be null or empty.'), 406);
    }
    
    $node = node_load($group_nid);
    if(isset($group_participant) && !empty($group_participant)) {
      $participants = '';
      unset($node->field_participant[LANGUAGE_NONE]);
      foreach($group_participant as $key => $participant) {
        $participants.= $participant.',';
      }
      $node->field_participant[LANGUAGE_NONE][0]['value'] = substr($participants,0,-1);
    } else {
      unset($node->field_participant[LANGUAGE_NONE]);
    }
    if(isset($group_admin) && !empty($group_admin)){
      $admins = '';
      unset($node->field_group_admin[LANGUAGE_NONE]);
      foreach($group_admin as $key => $admin) {
        $admins.= $admin.',';
      }
      $node->field_group_admin[LANGUAGE_NONE][0]['value'] = substr($admins,0,-1);
    }
    if(isset($group_new_participant) && !empty($group_new_participant)){
      foreach($group_new_participant as $key => $new_participant) {
        drupal_mail('loop_club_service', 'loop_club_add',$new_participant,$language=array(),$params,NULL);
      }
    }
    node_save($node);
    $response = new stdClass();
    $account = user_load($node->uid);
    $response->nid = $node->nid;
    $response->title = $node->title;
    $response->created_by = ($account->field_full_name[LANGUAGE_NONE])?$account->field_full_name[LANGUAGE_NONE][0]['value']:NULL;
    $response->created_on = date('jS F Y',$node->created);
    $response->description = $node->body[LANGUAGE_NONE][0]['value'];
    $response->group_image = file_create_url($node->field_group_image[LANGUAGE_NONE][0]['uri']);
    if(isset($node->field_participant[LANGUAGE_NONE]) && !empty($node->field_participant[LANGUAGE_NONE])){
      $participants_raw = $node->field_participant[LANGUAGE_NONE][0]['value'];
      $participants = explode(',',$participants_raw);
      foreach($participants as $participant) {
        $response->participants[] = array('email' => $participant,'role'=>'participant');
      }
    }
    if(isset($node->field_group_admin[LANGUAGE_NONE]) && !empty($node->field_group_admin[LANGUAGE_NONE])){
      $admins_raw = $node->field_group_admin[LANGUAGE_NONE][0]['value'];
      $admins = explode(',',$admins_raw);
      foreach($admins as $admin) {
        $response->participants[] = array('email' => $admin,'role'=>($account->mail == $admin)?'creator':'admin');
      }
    }
    sort($response->participants);
    return $response;
  } else {
    return services_error(t('Data cannot be empty'), 406);
  }
}

function loop_club_service_resource_group_retrieve($group_id){
  $node = node_load($group_id);
  $account = user_load($node->uid);
  $response->nid = $node->nid;
  $response->title = $node->title;
  $response->created_by = ($account->field_full_name[LANGUAGE_NONE])?$account->field_full_name[LANGUAGE_NONE][0]['value']:NULL;
  $response->updated_on = date('jS F Y',$node->changed);
  $response->created_on = date('jS F Y',$node->created);
  $response->description = $node->body[LANGUAGE_NONE][0]['value'];
  $response->fid = $node->field_group_image[LANGUAGE_NONE][0]['fid'];
  $response->group_image = file_create_url($node->field_group_image[LANGUAGE_NONE][0]['uri']);
  $response->group_closed = isset($node->field_group_closed)?$node->field_group_closed[LANGUAGE_NONE][0]['value']:0;
  $noParticipant = 0;
  
  if(isset($node->field_participant[LANGUAGE_NONE][0]['value']) && !empty($node->field_participant[LANGUAGE_NONE][0]['value'])){
    $participants_raw = $node->field_participant[LANGUAGE_NONE][0]['value'];
    $participants = explode(',',$participants_raw);
    foreach($participants as $participant) {
      $response->participants[] = array('email' => $participant,'role'=>'participant');
    }
  } else{
    $noParticipant = 1;
  }
  if(isset($node->field_group_admin[LANGUAGE_NONE][0]['value']) && !empty($node->field_group_admin[LANGUAGE_NONE][0]['value'])){
    $admins_raw = $node->field_group_admin[LANGUAGE_NONE][0]['value'];
    $admins = explode(',',$admins_raw);
    foreach($admins as $admin) {
      $response->participants[] = array('email' => $admin,'role'=>($account->mail == $admin)?'creator':'admin');
    }
    $noParticipant = 0;
  }
  else{
    $noParticipant = 1;
  }
  
  if($noParticipant == 1) {
    $response->participants = NULL;
  } else {
    sort($response->participants);
  }
  $get_topic_query = db_select('node','n')->extend('PagerDefault');
  $get_topic_query->leftjoin('field_data_field_group','fdfg','fdfg.entity_id = n.nid');
  $get_topic_query->leftjoin('field_data_body','fdb','fdb.entity_id = n.nid');
  $get_topic_query->leftJoin('comment','c','c.nid = n.nid AND c.status = 1');
  $get_topic_query->fields('n',array('title','nid','changed'));
  $get_topic_query->fields('fdb',array('body_value'));
  $get_topic_query->addExpression('COUNT(c.cid)','comment_count');
  $get_topic_query->limit(12);
  $get_topic_query->condition('fdfg.field_group_target_id',$group_id);
  $get_topic_query->condition('n.type','group_topic');
  $get_topic_query->condition('n.status',1);
  $get_topic_query->groupBy('n.nid');
  $get_topic_query->orderBy('n.changed','DESC');
  $group_topic = $get_topic_query->execute()->fetchAll();
  global $pager_total_items;
  if(isset($group_topic) && !empty($group_topic)) {
    $response->discussions = $group_topic;
    $response->total_discussions = $pager_total_items[0];
  } else {
    $response->discussions = NULL;
  }
  return $response;
}

function loop_club_service_resource_create_group_topic($data) {
  global $user;
  if(isset($data) && !empty($data)) {
    $topic_title = filter_xss($data['title']);
    $topic_description = filter_xss($data['description']);
    $topic_group = filter_xss($data['group_id']);
    
    if(!isset($topic_title) || empty($topic_title)) {
      return services_error(t('Topic Title cannot be null or empty.'), 406);
    }
    if(!isset($topic_group) || empty($topic_group)) {
      return services_error(t('Topic Group cannot be null or empty.'), 406);
    }
    
    $node = new stdClass();
    $node->title = $topic_title;
    $node->type = "group_topic";
    node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
    $node->language = LANGUAGE_NONE; // Or e.g. 'en' if locale is enabled
    $node->uid = $user->uid; 
    $node->status = 0; //(1 or 0): published or not
    $node->promote = 0; //(1 or 0): promoted to front page
    $node->comment = 1; // 0 = comments disabled, 1 = read only, 2 = read/write
    if(isset($topic_description) && !empty($topic_description)) {
      $node->body[LANGUAGE_NONE][0]['value'] = $topic_description;
    }
    // Group Participant - Entity reference field
    if(isset($topic_group) && !empty($topic_group)) {
      $node->field_group[$node->language][0] = array(
        'target_id' => $topic_group,
      );
    }
    $node = node_submit($node); // Prepare node for saving
    node_save($node);
    return t('Topic saved successfully');
  } else {
    return services_error(t('Data cannot be empty'), 406);
  }
}

function loop_club_service_resource_my_group_retrieve($data) {
  $email = filter_xss($data['mail']);
  global $base_url;
  $search = filter_xss($data['search']);
  $query = db_select('node','n')->extend('PagerDefault');
  $query->leftJoin('field_data_field_group_admin','ga','ga.entity_id = n.nid');
  $query->leftJoin('field_data_field_participant','gp','gp.entity_id = n.nid');
  $query->leftJoin('field_data_field_private_group','pg','pg.entity_id = n.nid');
  $query->leftJoin('field_data_field_group_image','gi','gi.entity_id = n.nid');
  $query->leftJoin('field_data_field_group_closed','gc','gc.entity_id = n.nid');
  $query->leftJoin('file_managed','fm','fm.fid = gi.field_group_image_fid');
  $query->leftJoin('field_data_field_full_name','fn','fn.entity_id = n.uid');
  $query->fields('n',array('nid','title'));
  $query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'group_image');
  $query->addField('fn','field_full_name_value','username');
  $query->limit(14);
  $query->condition('n.type','loop_club');
  $query->condition('n.status',1);
  $query->condition('gc.field_group_closed_value',0);
  $query->condition('pg.field_private_group_value',1);
  $db_or = db_or();
  $db_or->condition('ga.field_group_admin_value','%'.$email.'%','LIKE');
  $db_or->condition('gp.field_participant_value','%'.$email.'%','LIKE');
  $query->condition($db_or);
  if(isset($search) && !empty($search)){
    $query->condition('n.title','%'.$search.'%','LIKE');
  }
  $query->groupBy('n.nid');
  $query->orderBy('n.created','DESC');
  $record = $query->execute()->fetchAll();
  $response['groups'] = $record;
  global $pager_total_items;
  $response['total_comments'] = $pager_total_items[0];
  return $response;
}

function loop_club_service_resource_admin_group_retrieve() {
  global $base_url;
  $search = filter_xss($_GET['search']);
  $query = db_select('node','n')->extend('PagerDefault');
  $query->join('field_data_field_private_group','pg','pg.entity_id = n.nid');
  $query->leftJoin('field_data_body','fb','fb.entity_id = n.nid');
  $query->leftJoin('field_data_field_group_image','gi','gi.entity_id = n.nid');
  $query->leftJoin('file_managed','fm','fm.fid = gi.field_group_image_fid');
  $query->leftJoin('field_data_field_full_name','fn','fn.entity_id = n.uid');
  $query->fields('n',array('nid','title','created'));
  $query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'group_image');
  $query->addField('fn','field_full_name_value','username');
  $query->addField('fb','body_value');
  $query->limit(15);
  $query->condition('n.type','loop_club');
  $query->condition('n.status',1);
  $query->condition('pg.field_private_group_value',0);
  if(isset($search) && !empty($search)){
    $query->condition('n.title','%'.$search.'%','LIKE');
  }
  $query->groupBy('n.nid');
  $query->orderBy('n.created','DESC');
  $record = $query->execute()->fetchAll();
  $response['groups'] = $record;
  global $pager_total_items;
  $response['total_comments'] = $pager_total_items[0];
  return $response;
}

function loop_club_service_resource_group_topics_retrieve($topic_id) {
  global $base_url;
  $topic_node = node_load($topic_id);
  $response['title'] = $topic_node->title;

  if(isset($_GET['mobile']) && $_GET['mobile']==1) {
    $response['body_value'] = $topic_node->body[LANGUAGE_NONE][0]['value'];
  }
  else {
    $response['topic_description'] = $topic_node->body[LANGUAGE_NONE][0]['value'];
  }

  $account = user_load($topic_node->uid);
  $response['creator_name'] = ($account->field_full_name[LANGUAGE_NONE])?$account->field_full_name[LANGUAGE_NONE][0]['value']:NULL;
  $response['created'] = $topic_node->changed;
  
  $group_id = $topic_node->field_group[LANGUAGE_NONE][0]['target_id'];
  $group = node_load($group_id);
  $response['group_closed'] = isset($group->field_group_closed)?$group->field_group_closed[LANGUAGE_NONE][0]['value']:0;
  $response['private_group'] = isset($group->field_private_group)?$group->field_private_group[LANGUAGE_NONE][0]['value']:0;
  $query = db_select('comment','c')->extend('PagerDefault');
  $query->join('field_data_comment_body','cb','cb.entity_id = c.cid');
  $query->leftJoin('users','u','c.uid = u.uid');
  $query->leftJoin('field_data_field_full_name','fn','fn.entity_id = u.uid');  
  $query->leftJoin('file_managed','fm','fm.fid = u.picture');
  if(isset($_GET['mobile']) && $_GET['mobile']==1) {
    $query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'picture');
    $query->fields('c',array('subject','created','uid'));
    $query->addField('fn','field_full_name_value','author');
  }
  else {
    $query->addExpression('REPLACE(fm.uri,\'public://\',\''.$base_url.'/sites/default/files/\')', 'user_picture');
    $query->fields('c',array('subject','created','uid'));
    $query->addField('fn','field_full_name_value','username');
  }

  $query->addField('cb','comment_body_value','comment');
  $query->limit(10);
  $query->condition('c.status',1);
  $query->condition('c.nid',$topic_id);
  $query->orderBy('c.created','DESC');
  $record = $query->execute()->fetchAll();
  $response['comments'] = $record;
  global $pager_total_items;

  if(isset($_GET['mobile']) && $_GET['mobile']==1) {
    $response['comment_count'] = $pager_total_items[0];
  }
  else {
    $response['total_comments'] = $pager_total_items[0];
  }

  return $response;
}

function loop_club_service_resource_create_comment($data){
  global $user;
  
  $subject = filter_xss($data['subject']);
  $comment_value = filter_xss($data['comment']);
  $nid = filter_xss($data['topic_id']);
  
  if(!isset($subject) || empty($subject)) {
    return services_error(t('Comment Subject cannot be null or empty.'), 406);
  }
  if(!isset($comment_value) || empty($comment_value)) {
    return services_error(t('Comment cannot be null or empty.'), 406);
  }
  if(!isset($nid) || empty($nid)) {
    return services_error(t('Topic need to be selected.'), 406);
  }
  
  $comment = new stdClass();
  $comment->nid = $nid; // nid of a node you want to attach a comment to
  $comment->cid = 0; // leave it as is
  $comment->pid = 0; // parent comment id, 0 if none 
  $comment->uid = $user->uid; // user's id, who left the comment
  $comment->created = time(); // OPTIONAL. You can set any time you want here. Useful for backdated comments creation.
  $comment->is_anonymous = 0; // leave it as is
  $comment->status = COMMENT_NOT_PUBLISHED; // We auto-publish this comment
  $comment->language = LANGUAGE_NONE; // The same as for a node
  $comment->subject = $subject; 
  $comment->comment_body[$comment->language][0]['value'] = $comment_value; // Everything here is pretty much like with a node
  $comment->comment_body[$comment->language][0]['format'] = 'filtered_html'; 
  #echo "<pre>" .print_r($comment,"\n"). "</pre>"; exit;
  comment_submit($comment); // saving a comment
  comment_save($comment);
  return t('Your comment is submitted successfully, you can view the comment after it is been moderated.');
}

function loop_club_service_resource_delete_group($data) {
    $group_id = filter_xss($data['nid']);
    if(isset($group_id) && !empty($group_id)) {
      $node = node_load($group_id);
      $node->field_group_closed[LANGUAGE_NONE][0]['value'] = 1;
      $node = node_submit($node); // Prepare node for saving
      node_save($node);
      $params['group_name'] = $node->title;    
      if(isset($node->field_participant[LANGUAGE_NONE]) && !empty($node->field_participant[LANGUAGE_NONE])){
        $participants_raw = $node->field_participant[LANGUAGE_NONE][0]['value'];
        $participants = explode(',',$participants_raw);
        foreach($participants as $participant) {
          drupal_mail('loop_club_service', 'loop_club_delete',$participant,$language=array(),$params,NULL);
        }
      }
      if(isset($node->field_group_admin[LANGUAGE_NONE]) && !empty($node->field_group_admin[LANGUAGE_NONE])){
        $admins_raw = $node->field_group_admin[LANGUAGE_NONE][0]['value'];
        $admins = explode(',',$admins_raw);
        foreach($admins as $admin) {
          drupal_mail('loop_club_service', 'loop_club_delete',$admin,$language=array(),$params,NULL);
        }
      }
      return t('Group has been closed.');
    } else {
      return services_error(t('There is no group information.'), 406);
    }
}