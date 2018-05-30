<?php
function _services_search_resource_node() {
  // Invoke the search hook to generate results.
  $keys = filter_xss($_GET['search']);
  $results = module_invoke('node', 'search_execute', $keys);
  global $pager_total_items;
  $i = 0;
  if ($results && is_array($results) && count($results)) {
    foreach ($results as &$result) {
      $node = $result['node'];
      #echo "<pre>" .print_r($node,"\n"). "</pre>";
      if($node->type == 'article') {
        // The 'date' attribute returned by node_search_execute() represents the
        // node's 'updated' timestamp. Also add the 'created' timestamp.
        $response[$i]->created = date("F d,Y",$node->created);
        $response[$i]->title = $node->title;
        $response[$i]->type = $node->type;
        $response[$i]->intro = $node->field_intro[LANGUAGE_NONE][0]['value'];
        $response[$i]->thumbnail = 	file_create_url($node->field_square_variant_image[LANGUAGE_NONE][0]['uri']);
        $category = $node->field_primary_category[LANGUAGE_NONE][0]['taxonomy_term'];
        $response[$i]->primary_category = $category->name;
        $response[$i]->color = $category->field_color['und'][0]['rgb'];
        $response[$i]->nid = $node->nid;
        $response[$i]->source = $node->field_source[LANGUAGE_NONE][0]['value'];
        switch($response[$i]->source) {
          case 'user':
            if($node->field_select_user_or_student[LANGUAGE_NONE][0]['value'] == 'user') {
              if(isset($node->field_user_name) && !empty($node->field_user_name)){
                $user_value = $node->field_user_name[LANGUAGE_NONE][0]['user'];
                $response[$i]->source_name = $user_value->field_full_name[LANGUAGE_NONE][0]['value'];
              }
            } else if($node->field_select_user_or_student[LANGUAGE_NONE][0]['value'] == 'student') {
              if(isset($node->field_student_name) && !empty($node->field_student_name)){
                $student_value = $node->field_student_name[LANGUAGE_NONE][0]['node'];
                $response[$i]->source_name = $student_value->title;
              }
            }
          break;
          case 'expert':
            if(isset($node->field_expert_name) && !empty($node->field_expert_name)){
              $expert_value = $node->field_expert_name[LANGUAGE_NONE][0]['node'];
              $response[$i]->source_name = $expert_value->title;
            }
          break;
          case 'admin':
            if(isset($node->field_loop_staff_name) && !empty($node->field_loop_staff_name)){
              $loop_staff_value = $node->field_loop_staff_name[LANGUAGE_NONE][0]['node'];
              $response[$i]->source_name = $loop_staff_value->title;
            }  
          break;
          case 'custom_byline':
            $response[$i]->by_line = $node->field_by_line[LANGUAGE_NONE][0]['value'];
          break;
        }
        $i++;
      }
    }
    /*$PageNo = filter_xss($_GET['page']);
    $RowsPerPage = 10;
    $result_count = count($response);
    $LastPage     = ceil($result_count/$RowsPerPage);
    $PageNo = (int)$PageNo;
    
    if ($PageNo <= 1 || $PageNo == "")
    {
       $PageNo = 1;
    }
    elseif ($PageNo > $LastPage)
    {
       $PageNo = $LastPage;
    }
    $records = array_slice($response,($PageNo - 1) * $RowsPerPage,$RowsPerPage);*/
    $response['total_result'] = (string)$pager_total_items[0];
    return $response;
  }
  return services_error(t('Search returned no results.'), 404);
}
