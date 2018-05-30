<?php
print '<pre>';
//$output = shell_exec("rm -rvf ae/*");
//$output = shell_exec("tar -zcvf khaleej_content.tar.gz ../loop --exclude khaleej_content.tar.gz");
//$output = shell_exec("tar -xvf  drupal-8.tar.gz");

//$output = shell_exec("tar -zxvf databasesep15005177721023438_live_hb.tar.gz");

//$output = shell_exec("rm -rf ../delthis");

// $output = shell_exec("ln -s ../content bh");
// $output = shell_exec("ln -s ../content ae");
// $output = shell_exec("ln -s ../content sa");

//$output = shell_exec('unzip drupal-8.zip');

//$output = shell_exec('wget http://blog.homecentre.com/sa/backup/media_filessep1479735852.tar.gz');

/*
Old configuration::
*/


$host       = 'localhost';
$db = 'khaleej';
$user =  'root';
$pass       = 'khaleej$123';

$output = shell_exec('mysqldump -h '. $host . ' -u '. $user . ' -p'. $pass . ' '. $db . ' > khaleej_live_db.sql');


/*
New configuration:
*/
// $host       = 'mariadb-100.wc2.dfw3.stabletransit.com';
// $db = '1023438_home_box';
// $user =  '1023438_homebox';
// $pass       = '7LekQ3BUtB2bq8ta';

// //print "--\n";

// //$output = shell_exec('mysql -h '. $host . ' -u '. $user . ' -p'. $pass . '  '. $db . ' < ./1023438_dev_hb.sql');
// $cmd = 'mysql --host='. $host . ' --user='. $user . ' --password='. $pass . ' --database='. $db . ' < 1023438_dev_hb.sql';
// $output = shell_exec($cmd);
//$output = shell_exec("ln -s ../contentd7 bs_app"); 

//$output = shell_exec("unzip sites/default/files/JPEG-OCT.zip -d sites/default/files/JPEG");


//$output = shell_exec("mv  -v sites/default/files/JPEG/JPEG/* sites/default/files/JPEG/");


print_r($output);


