<?php

/**
 * @file
 * Theme functions for fileupload.
 */

/**
 * Theme the list of unscored fileupload questions.
 */
function theme_quizfileupload_view_unscored($variables) {
  $unscored = $variables['unscored'];
  $output = '';
  $header = array(
    t('Question'),
    t('Time Finished'),
    t('Action')
  );
  $rows = array();

  foreach ($unscored as $item) {
    if ($item->time_end > 0) {
      $rows[] = array(
        $item->title,
        date('Y-m-d H:i', $item->time_end),
        l(t('score this response'), 'admin/quiz/reports/score-fileupload/' . $item->question_vid . '/' . $item->result_id, array('query' => drupal_get_destination())),
      );
    }
  }

  if (!empty($rows)) {
    $output .= theme('table', array('header' => $header, 'rows' => $rows));
  }
  else {
    $output .= t('There are no unscored fileuploads.');
  }
  return $output;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_quizfileupload_user_answer($variables) {
  $answer = $variables['answer'];
  $correct = $variables['correct'];
  $header = array(t('Correct Answer'), t('User Answer'));
  $row = array(array($correct, $answer));
  return theme('table', array('header' => $header, 'rows' => $row));
}

/**
 * Theme the quizfileupload response form
 *
 * @param $form
 *  The response form
 */
function theme_quizfileupload_response_form($variables) {
  $form = $variables['form'];
  return drupal_render_children($form);
}
