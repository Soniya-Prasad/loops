<?php
define('DRUPAL_ROOT', getcwd());
ini_set('display_errors', 1);
error_reporting(E_ALL);
require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

if((isset($_GET['nid']) && is_numeric($_GET['nid'])) && (isset($_GET['status']) && is_numeric($_GET['status']))) {
$url = FRONT_PAGE_PATH. '/php_script/articles.php?nid='.$_GET['nid'].'&status='.$_GET['status'];
drupal_http_request($url);
drupal_goto(FRONT_PAGE_PATH.'/article-preview.html',array('query'=>array('nid' => $_GET['nid'],'status'=>$_GET['status'])));
}?>