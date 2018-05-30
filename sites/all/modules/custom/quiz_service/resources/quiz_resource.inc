<?php
/**
 * Callback for retrieving the quiz to be taken.
 * $quiz_id - Quiz nid. 
 */ 
function quiz_resource_retrieve($quiz_id){
  global $user;
  $response = array();
  $quiz = node_load($quiz_id);
  if(isset($quiz) && !empty($quiz)) {
    $response['quiz_name'] = $quiz->title;
    $response['quiz_id'] = $quiz_id;
    $response['type'] = $quiz->type;
    $response['created'] = $quiz->created;
    $response['changed'] = $quiz->changed;
    $response['content'] = $quiz->body[LANGUAGE_NONE][0]['value'];
    $response['quiz_detail']['quiz_open'] = $quiz->quiz->quiz_open;
    $response['quiz_detail']['quiz_close'] = $quiz->quiz->quiz_close;
    $response['quiz_detail']['quiz_always'] = $quiz->quiz->quiz_always;
    $response['quiz_detail']['time_limit'] = $quiz->quiz->time_limit;
    $response['quiz_detail']['pass_percentage'] = $quiz->quiz->pass_rate;
    $response['quiz_detail']['pass_message'] = $quiz->quiz->summary_pass;
    $response['quiz_detail']['fail_message'] = $quiz->quiz->summary_default;
    $response['quiz_detail']['competition_venue'] = $quiz->field_venue[LANGUAGE_NONE][0]['value'];
    $response['quiz_detail']['competition_date'] = $quiz->field_competition_date[LANGUAGE_NONE][0]['value'];
    $result = quiz_access('take', $quiz);
    if (!$result['success']) {
      return services_error($result['message'], 406);
    }
    else if (!empty($result['message'])) {
      // There's a friendly message available.
      return services_error($result['message'], 406);
    }
  
    if ($quiz_result = quiz_take_quiz($quiz)) {
      if (!empty($quiz_result->resume)) {
        
      }
    }
    else {
      if ($result['success']) {
        $quiz_result = entity_create('quiz_result', array(
          'nid' => $quiz->nid,
          'vid' => $quiz->vid,
          'uid' => $GLOBALS['user']->uid,
          'type' => $quiz->quiz->result_type,
        ));
      }
      else {
        return services_error($result['message'], 406);
      }
    }
  
    $build_on_last = !empty($quiz->build_on_last) && QuizResultController::findOldResult($quiz_result);
    if ($build_on_last || $instances = field_info_instances('quiz_result', $quiz->quiz->result_type)) {
      // @kludge above, how are we going to check this form for fields? checking
      // for field instances is easy, but what about these one-offs? maybe we can
      // require add-on field items to put something in the $form so that we can
      // check it. I don't want to have the "start" button show if we don't have
      // anything to ask the user.
      //return entity_ui_get_form('quiz_result', $quiz_result);
    }
    else {
      // New attempt.
      $quiz_result->save();
      $_SESSION['quiz'][$quiz_result->nid]['result_id'] = $quiz_result->result_id;
      $_SESSION['quiz'][$quiz_result->nid]['current'] = 1;
    }
    $response['quiz_detail']['time_start'] = (string)$quiz_result->time_start;
    $response['quiz_detail']['type'] = $quiz_result->type;
    $result_id = $quiz_result->result_id;
    if (!empty($_SESSION['quiz'][$quiz->nid]['result_id'])) {
      $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
      $layout = $quiz_result->getLayout();
      $response['result_id'] = $quiz_result->result_id;

      if(!empty($layout)) {
        foreach($layout as $key => $question) {
        
          $qra = quiz_result_answer_load($result_id, $question['nid'], $question['vid']);
          $current_question = node_load($question['nid'], $question['vid']);
          
          foreach ($layout as $question_new) {
            if ($question['nid'] == $current_question->nid) {
              $question_array = $question_new;
            }
          }
          
          if (!$qra) {
          // This is the same skip code as in quiz_question_answering_form_submit() @todo find a way to save code.
          $qi_instance = _quiz_question_response_get_instance($result_id, $current_question, NULL);
          $qi_instance->delete();
          $bare_object = $qi_instance->toBareObject();
          quiz_store_question_result($quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
          }
             
          $question_detail = node_load($question['nid'],$question['vid']);
          #echo "<pre>" .print_r($question_detail,"\n"). "</pre>"; 
          $result_answer_id = db_query("SELECT result_answer_id
                FROM {quiz_node_results_answers}
                WHERE question_nid = :question_nid
                AND question_vid = :question_vid
                AND result_id = :result_id", array(':question_nid' => $question_detail->nid, ':question_vid' => $question_detail->vid, ':result_id' => $result_id))->fetchField();

          
          $response['quiz_question'][$key]['question'] =  $question_detail->body[LANGUAGE_NONE][0]['value'];
          $response['quiz_question'][$key]['nid'] =  $question_detail->nid;
          $response['quiz_question'][$key]['result_answer_id'] =  $result_answer_id;
          $response['quiz_question'][$key]['created'] =  $question_detail->created;
          $response['quiz_question'][$key]['changed'] =  $question_detail->changed;
          $response['quiz_question'][$key]['question_type'] =  $question_detail->type;
          switch($question_detail->type){
            case 'multichoice':
              $response['quiz_question'][$key]['choice_multi'] = $question_detail->choice_multi;
              foreach($question_detail->alternatives as $mkey => $multichoice){
                $response['quiz_question'][$key]['multichoice'][$mkey]['id'] = $multichoice['id'];
                $multichoice['answer']['value'] = str_replace("\r\n","",$multichoice['answer']['value']);
                $response['quiz_question'][$key]['multichoice'][$mkey]['answers'] = strip_tags($multichoice['answer']['value']);
                $response['quiz_question'][$key]['multichoice'][$mkey]['weight'] = $multichoice['weight'];
                
              }  
            break;
            case 'quizfileupload':
              $response['quiz_question'][$key]['filetypes'] = $question_detail->filetypes;
            break;
          }
          if(isset($_GET['mobile']) && $_GET['mobile']==1){
          $response['quiz_question'] = array_values($response['quiz_question']);
          }

        }
      }
      else {
        $response['quiz_question'] = null;
      }
      
      return $response;
    }
  }
  else {
    return services_error(t('Invalid Quiz.'), 406);
  }
}
/**
 * Callback for submitting the quiz taken by the user.
 */ 
function quiz_resource_submit_quiz($data) {
  watchdog('submit_quiz','@submit',array('@submit'=> print_r($data,true)));
  global $user;
  $result = quiz_result_load($data['result_id']);
  if(empty($result->time_end) && $user->uid == $result->uid){
    foreach($data['quiz_answers'] as $answers) {
      if(isset($_GET['mobile']) && $_GET['mobile']==1){
      	$answers['answer'] = $answers['answerString'];
      }
      switch($answers['question_type']) {
        case 'long_answer':
          if(isset($answers['answer']) && !empty($answers['answer'])) {
            db_delete('quiz_long_answer_user_answers')->condition('result_answer_id',$answers['result_answer_id'])->execute();
            db_insert('quiz_long_answer_user_answers')
            ->fields(array(
              'result_answer_id' => $answers['result_answer_id'],
              'score' => 0,
              'is_evaluated' => 0,
              'answer' => $answers['answer'],
              'answer_format' => 'plain_text',
            ))->execute();
          }
        break;
        case 'multichoice':
          if(isset($answers['multichoice_answer_id']) && !empty($answers['multichoice_answer_id'])) {
            $multi_answer_id = db_select('quiz_multichoice_user_answers','mua')->fields('mua',array('id'))->condition('result_answer_id',$answers['result_answer_id'])->execute()->fetchField();
            
            db_delete('quiz_multichoice_user_answer_multi')->condition('user_answer_id',$multi_answer_id)->execute();
            
            db_delete('quiz_multichoice_user_answers')->condition('result_answer_id',$answers['result_answer_id'])->execute();
            $multichoice_answer_id = db_insert('quiz_multichoice_user_answers')
            ->fields(array(
              'result_answer_id' => $answers['result_answer_id'],
            ))->execute();
            foreach($answers['multichoice_answer_id'] as $choice) {
              db_insert('quiz_multichoice_user_answer_multi')->fields(array('user_answer_id' => $multichoice_answer_id,'answer_id' => $choice))->execute();
            }
          }
        break;
        case 'truefalse':
          if(($answers['answer'] === 0) || ($answers['answer'] === 1)) {
            db_delete('quiz_truefalse_user_answers')->condition('result_answer_id',$answers['result_answer_id'])->execute();
            db_insert('quiz_truefalse_user_answers')->fields(array('result_answer_id'=>$answers['result_answer_id'],'answer'=>$answers['answer'],'score'=>0))->execute();
          }
        break;
        case 'quizfileupload':
          if(isset($answers['fid']) && !empty($answers['fid'])) {
            db_delete('quiz_fileupload_user_answers')->condition('result_answer_id',$answers['result_answer_id'])->execute();
            db_insert('quiz_fileupload_user_answers')
            ->fields(array(
              'result_answer_id' => $answers['result_answer_id'],
              'score' => ($answers['fid'])?1:0,
              'is_evaluated' => 1,
              'fid' => $answers['fid']
            ))->execute();
          }
        break;
        default:
          return services_error(t('Question Type Doesn\'t Support'), 406);
        break;
      }
    }
    
    /** Calculate Score - Start **/
    $get_answers = _quiz_get_answers($data['quiz_id'],$data['result_id']);
    $total_score = '';
    $score = '';
    $final_score = '';
    foreach($get_answers as $nid => $ans) {
      $score+= $ans->answers[0]['score'];
      $total_score+= $ans->qnr_max_score;
      db_update('quiz_node_results_answers')->fields(array('is_correct'=>(($ans->answers[0]['is_correct'])?$ans->answers[0]['is_correct']:0),'points_awarded'=>(($ans->answers[0]['score'])?$ans->answers[0]['score']:0),'answer_timestamp'=>time()))->condition('question_nid',$ans->nid)->condition('question_vid',$ans->vid)->condition('result_id',$data['result_id'])->execute();
    }
    $final_score = round(($score / $total_score)*100);
    /** Calculate Score - End **/
    $end_time = time();
    if(isset($data['end_time']) && !empty($data['end_time'])){
      $end_time = $data['end_time'];
    }
    db_update('quiz_node_results')->fields(array('time_end'=>time(),'finish_time'=>$end_time,'score'=>$final_score))->condition('result_id',$data['result_id'])->execute();
    unset($_SESSION['quiz']);
    
    
    $display_result = db_select('field_data_field_display_result','d')->fields('d',array('field_display_result_value'))->condition('entity_id',$data['quiz_id'])->condition('bundle','quiz')->execute()->fetchField();
    
    $response->message = t('Quiz submitted successfully.');
    $quiz_name = db_select('node','n')->fields('n',array('title'))->condition('type','quiz')->condition('nid',$data['quiz_id'])->execute()->fetchField();
    if($quiz_name) {
      $response->quiz_name = $quiz_name;
    }
    if($display_result) {
      $get_result = _quiz_get_answers($data['quiz_id'],$data['result_id']);
      $i=1;
      foreach($get_result as $nid => $result_values) {
        $myresult[$i]->question = $result_values->body[LANGUAGE_NONE][0]['value'];
        $myresult[$i]->question_type = $result_values->type;
        $myresult[$i]->max_score = $result_values->qnr_max_score;
        $myresult[$i]->weight = $i;
        
        switch($result_values->type)
        {
          case 'long_answer':
           $long_answer = str_replace("\r\n","",$result_values->rubric['value']);
           $long_answer = strip_tags($long_answer);
            $myresult[$i]->correct_answer = array($long_answer);
          break;
          case 'multichoice':
            $myresult[$i]->choice_multi = $result_values->choice_multi;
            
              $youranswer = db_query('SELECT uam.* FROM quiz_node_results_answers ra INNER JOIN quiz_multichoice_user_answers ua ON ua.result_answer_id = ra.result_answer_id INNER JOIN quiz_multichoice_user_answer_multi uam ON uam.user_answer_id = ua.id where ra.result_id = '.$data['result_id'].' and ra.question_nid = '.$result_values->answers[0]['question_nid'].' and ra.question_vid = '.$result_values->answers[0]['question_vid'])->fetchAllKeyed(1);
            $options = array();
            $option_answer = array();
            foreach($result_values->alternatives as $key => $option) {
              if($option['score_if_chosen'] == 1){
                $option['answer']['value'] = str_replace("\r\n","",$option['answer']['value']);
                $options[] = strip_tags($option['answer']['value']);
              }
              if(in_array($option['id'],$youranswer)) {
                $option['answer']['value'] = str_replace("\r\n","",$option['answer']['value']);
                $option_answer[] = strip_tags($option['answer']['value']);
              }
            }
            $myresult[$i]->correct_answer = $options;
            $myresult[$i]->your_answer = $option_answer;
          break;
          case 'truefalse':
            $myresult[$i]->correct_answer = array($result_values->correct_answer);
          break;
          case 'quizfileupload':
            $answerfile = $result_values->answers[0]['answer'];
            if(isset($answerfile) && !empty($answerfile)) {
              $file = file_load($answerfile);
              $myresult[$i]->your_answer = array(file_create_url($file->uri));
            }
          break;
        }
        foreach($result_values->answers as $answers){
            if($result_values->type != 'multichoice' && $result_values->type != 'quizfileupload') {
               if(isset($_GET['mobile']) && $_GET['mobile']==1){
                $myresult[$i]->your_answer = array($answers['answer']);
               }else{
                $myresult[$i]->your_answer = array($answers['answer']);
                }
            }
            $myresult[$i]->is_correct = ($answers['is_correct'])?$answers['is_correct']:false;
            $myresult[$i]->score = ($answers['score'])?$answers['score']:0;
            $myresult[$i]->is_evaluated = ($answers['is_evaluated'])?$answers['is_evaluated']:false;
            
        }
        $i++;
      } 
       if(isset($_GET['mobile']) && $_GET['mobile']==1){
        $response->result = array_values($myresult);
       }
       else{
        $response->result = $myresult;
       }
    }
    return $response;
  } else {
    return services_error(t('You cannot attempt the quiz again'), 406);
  }
}

function quiz_result_resource_retrieve($quiz_id, $result_id) {
  //echo $quiz_id . " - " . $result_id;
  $get_answers = _quiz_get_answers($quiz_id,$result_id);
}