<?php

/**
 * The main classes for the quizfileupload question type.
 *
 * These inherit or implement code found in quiz_question.classes.inc.
 *
 * Based on:
 * Other question types in the quiz framework.
 *
 * @file quizfileupload.classes.inc
 * Question implementation for uploading files.
 */

/**
 * Extension of QuizQuestion.
 */
class QuizfileuploadQuestion extends QuizQuestion {

  // Constants for answer matching options
  const ANSWER_MATCH = 0;
  const ANSWER_MANUAL = 1;

  /**
   * Implements saveNodeProperties().
   *
   * @see QuizQuestion#saveNodeProperties($is_new)
   */
  public function saveNodeProperties($is_new = FALSE) {
    db_merge('quiz_fileupload_node_properties')
      ->key(array(
        'nid' => $this->node->nid,
        'vid' => $this->node->vid,
      ))
      ->fields(array(
        'nid' => $this->node->nid,
        'vid' => $this->node->vid,
        'filetypes' => $this->node->filetypes,
        'correct_answer_evaluation' => $this->node->correct_answer_evaluation,
      ))
      ->execute();
  }

  /**
   * Implements validateNode().
   *
   * @see QuizQuestion#validateNode($form)
   */
  public function validateNode(array &$form) {
    //no validation required
  }

  /**
   * Implements delete().
   *
   * @see QuizQuestion#delete($only_this_version)
   */
  public function delete($only_this_version = FALSE) {
    parent::delete($only_this_version);

    $delete_node = db_delete('quiz_fileupload_node_properties');
    $delete_node->condition('nid', $this->node->nid);

    if ($only_this_version) {
      $delete_node->condition('vid', $this->node->vid);
    }

    $delete_node->execute();
  }

  /**
   * Implements getNodeProperties().
   *
   * @see QuizQuestion#getNodeProperties()
   */
  public function getNodeProperties() {
    if (isset($this->nodeProperties)) {
      return $this->nodeProperties;
    }
    $props = parent::getNodeProperties();

    // Load the properties
    $result = db_select('quiz_fileupload_node_properties', 'qfnp')
      ->fields('qfnp', array('filetypes', 'correct_answer_evaluation'))
      ->condition('nid', $this->node->nid, '=')
      ->condition('vid', $this->node->vid, '=')
      ->execute()
      ->fetchAssoc();

    $this->nodeProperties = (is_array($result)) ? array_merge($props, $result) : $props;
    return $this->nodeProperties;
  }

  /**
   * Implements getNodeView().
   *
   * @see QuizQuestion#getNodeView()
   */
  public function getNodeView() {
    $content = parent::getNodeView();

    // No need to really check permissions as we actually don't show any answer here.
    //if ($this->viewCanRevealCorrect()) {
    $content['answers'] = array(
      '#markup' => '<div class="quiz-solution">' . check_plain($this->node->filetypes) . '</div>',
      '#weight' => 2,
    );
    return $content;
  }

  /**
   * Implements getAnsweringForm().
   *
   * @see QuizQuestion#getAnsweringForm($form_state, $result_id)
   */
  public function getAnsweringForm(array $form_state = NULL, $result_id) {
    $element = parent::getAnsweringForm($form_state, $result_id);

    $element += array(
      '#type' => 'managed_file',
      '#title' => t('Upload'),
      '#description' => t('Allowed extensions !ext', array('!ext' => $this->node->filetypes)),
      '#required' => FALSE,
      '#upload_location' => variable_get('quizfileupload_upload_location', 'public') . '://quizfileupload/' . $this->node->nid . '/',
      '#upload_validators' => array(
        'file_validate_extensions' => array($this->node->filetypes),
      ),
    );

    if (isset($result_id)) {
      $response = new QuizfileuploadResponse($result_id, $this->node);
      $element['#default_value'] = $response->getResponse();
    }

    return $element;
  }

  /**
   * Question response validator.
   */
  public function getAnsweringFormValidate(array &$element, &$value) {
    if (empty($value['fid'])) {
      form_set_error($element['#name'], t('You must upload a file.'));
    }
    else {
      // If we have a file we'll set the status to permanent.
      // We mainly do it here so that the "allow resume" function of the quiz
      // should work as expected for the file upload as well.
      $file = file_load($value['fid']);
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
    }
  }

  /**
   * Implements getCreationForm().
   *
   * @see QuizQuestion#getCreationForm()
   */
  public function getCreationForm(array &$form_state = NULL) {
    $allowed = variable_get('quizfileupload_default_extensions', QUIZFILEUPLOAD_DEFAULT_EXTENSIONS);
    $form['filetypes'] = array(
      '#type' => 'textfield',
      '#title' => t('Allowed extension'),
      '#description' => t('Enter the allowed file extensions one per line.'),
      '#default_value' => isset($this->node->filetypes) ? $this->node->filetypes : $allowed,
      '#required' => TRUE,
    );

    $options = array(
      self::ANSWER_MATCH => t('Automatic'),
      self::ANSWER_MANUAL => t('Manual'),
    );

    $form['correct_answer_evaluation'] = array(
      '#type' => 'radios',
      '#title' => t('Pick an evaluation method'),
      '#description' => t('Choose how the answer shall be evaluated.'),
      '#options' => $options,
      '#default_value' => isset($this->node->correct_answer_evaluation) ? $this->node->correct_answer_evaluation : self::ANSWER_MATCH,
      '#required' => TRUE,
    );

    return $form;
  }

  /**
   * Implements getMaximumScore().
   *
   * @see QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore() {
    return variable_get('quizfileupload_default_score', 1);
  }
}

/**
 * Extension of QuizQuestionResponse
 */
class QuizfileuploadResponse extends QuizQuestionResponse {
  /**
   * ID of the answer.
   */
  protected $answer_id = 0;
  protected $fid = NULL;
  private $answer_feedback;
  private $answer_feedback_format;

  /**
   * Constructor
   */
  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    if (!isset($answer)) {
      $r = db_select('quiz_fileupload_user_answers', 'qfua')
        ->fields('qfua', array('fid', 'score', 'is_evaluated', 'answer_id', 'answer_feedback', 'answer_feedback_format'))
        ->condition('result_answer_id', $this->result_answer_id, '=')
        ->execute()
        ->fetchObject();

      if (!empty($r)) {
        $this->answer = $r->fid;
        $this->score = $r->score;
        $this->evaluated = $r->is_evaluated;
        $this->answer_id = $r->answer_id;
        $this->answer_feedback = $r->answer_feedback;
        $this->answer_feedback_format = $r->answer_feedback_format;
      }
    }
    else {
      if (is_array($answer)) {
        $this->answer = $answer['fid'];
      }
      else {
        $this->answer = $answer;
      }
    }
  }

  /**
   * Get all quiz scores that haven't been evaluated yet.
   *
   * @param $count
   *  Number of items to return (default: 50).
   * @param $offset
   *  Where in the results we should start (default: 0).
   *
   * @return
   *  Array of objects describing unanswered questions. Each object will have result_id, question_nid, and question_vid.
   */
  public static function fetchAllUnscoredAnswers($count = 50, $offset = 0) {
    global $user;
    $query = db_select('quiz_fileupload_user_answers', 'qfua');
    $query->join('quiz_node_results_answers', 'qnra', 'qfua.result_answer_id = qnra.result_answer_id');
    $query->join('node_revision', 'nr', 'qnra.question_vid = nr.vid');
    $query->join('quiz_node_results', 'qnr', 'qnra.result_id = qnr.result_id');
    $query->join('node', 'n', 'qnr.nid = n.nid');
    $query->fields('qfua', array('answer_feedback', 'answer_feedback_format'));
    $query->fields('nr', array('title'));
    $query->fields('qnr', array('time_end', 'time_start', 'uid'));
    $query->fields('qnra', array('result_id', 'question_nid', 'question_vid'));
    $query->condition('qfua.is_evaluated', 0);

    if (user_access('score own quiz') && user_access('score taken quiz answer')) {
      $query->condition(db_or()->condition('n.uid', $user->uid)->condition('qnr.uid', $user->uid));
    }
    elseif (user_access('score own quiz')) {
      $query->condition('n.uid', $user->uid);
    }
    elseif (user_access('score taken quiz answer')) {
      $query->condition('qnr.uid', $user->uid);
    }

    $results = $query->execute();
    $unscored = array();
    foreach ($results as $row) {
      $unscored[] = $row;
    }
    return $unscored;
  }

  /**
   * Implements save().
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    // Check if we need to manually evaluate this
    $this->is_evaluated = (isset($this->question->correct_answer_evaluation) && $this->question->correct_answer_evaluation == 1) ? 0 : 1;
    db_merge('quiz_fileupload_user_answers')
      ->key(array('result_answer_id' => $this->result_answer_id))
      ->fields(array(
        'fid' => $this->answer,
        'result_answer_id' => $this->result_answer_id,
        'score' => $this->getScore(FALSE),
        'is_evaluated' => $this->is_evaluated,
      ))
      ->execute();
  }

  /**
   * Implements delete().
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    db_delete('quiz_fileupload_user_answers')
      ->condition('result_answer_id', $this->result_answer_id)
      ->execute();
  }

  /**
   * Implements score().
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    $score = 0;
    if (isset($this->result_answer_id)) {
      // Manual scoring means we go with what is in the DB.
      if (isset($this->question->correct_answer_evaluation) && $this->question->correct_answer_evaluation == 1) {
        $score = db_select('quiz_fileupload_user_answers', 'qfua')
          ->fields('qfua', array('score'))
          ->condition('result_answer_id', $this->result_answer_id, '=')
          ->execute()->fetchField();
        if (!$score) {
          $score = 0;
        }
      }
      // Otherwise, we run the scoring automatically.
      else {
        $fileuploadAnswer = new QuizfileuploadQuestion($this->question);
        $score = $fileuploadAnswer->getMaximumScore();
      }
    }
    return $score;
  }

  /**
   * Implements getResponse().
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    return $this->answer;
  }

  /**
   * Implements getFeedbackValues().
   */
  public function getFeedbackValues() {
    $data = array();

    // Hide the columns 'choice' and 'solution' as neither are really relevant for a fileupload question.
    $data[] = array(
        'choice' => NULL,
        'attempt' => quiz_file_markup($this->answer),
        'correct' => $this->question->answers[0]['is_correct'] ? quiz_icon('correct') : quiz_icon(''),
        'score' => !$this->evaluated ? t('This answer has not yet been scored.') : $this->getScore(),
        'answer_feedback' => check_markup($this->answer_feedback, $this->answer_feedback_format),
        'solution' => NULL,
    );

    return $data;
  }

  /**
   * Implements getReportFormAnswerFeedback().
   */
  public function getReportFormAnswerFeedback() {
    return array(
      '#title' => t('Enter feedback'),
      '#type' => 'text_format',
      '#default_value' => $this->answer_feedback,
      '#format' => isset($this->answer_feedback_format) ? $this->answer_feedback_format : filter_default_format(),
      '#attributes' => array('class' => array('quiz-report-score')),
    );
  }

  /**
   * Implements getReportFormSubmit
   *
   * @see QuizQuestionResponse#getReportFormSubmit()
   */
  public function getReportFormSubmit() {
    return 'quizfileupload_report_submit';
  }
}
