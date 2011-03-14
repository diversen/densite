<?php
 
include_once "curl.inc";

// your site
$site_url = 'http://drupal';
 
// profile
$profile = 'default';
 
// locale
$locale = 'en';
 
// database
$db_path = 'drupal';
 
// database user
$db_user = 'root';
 
// database pass
$db_pass = 'password';
 
// database host
$db_host = 'localhost';
 
// name of site
$site_name = 'site_name';
 
// site email
$site_mail = 'site_mail@site.com';
 
// admin account name
$account_name = 'admin';
 
// admin account email
$account_mail = 'admin@admin.dk';
 
// admin account pass
$account_pass = 'admin1234';
 
// date time zone
$date_default_timezone = '-39600';
 
// use clean url.
$clean_url = '1';
 
// update status module
$update_status_module = '1';
 
// end of settings
 // create a url for curling db settings
$url_str = "db_path=$db_path&";
$url_str.= "db_user=$db_user&";
$url_str.= "db_pass=$db_pass&";
$url_str.= "db_host=$db_host&";
$url_str.= "db_prefix=&";
$url_str.= "db_port=&";
$url_str.= "op=Save and continue" . "&";
$url_str.= "form_id=install_settings_form";
$url_str = urlencode($url_str);

$url = "$site_url/install.php?profile=$profile";
$curl = new mycurl($url, true);
$curl->createCurl();
echo $curl;

$url = "$site_url/install.php?profile=$profile&locale=$locale";
$curl = new mycurl($url, true);
$curl->setPost($url_str);
$curl->createCurl();
echo $curl;

$url = "$site_url/install.php?profile=$profile&locale=$locale&op=do_nojs&id=1";

//$url = "$site_url/install.php?profile=$profile&locale=$locale";
$curl = new mycurl($url, true);
$curl->setPost($url_str);
$curl->createCurl();
echo $curl;

//$site_url/install.php?profile=$profile&locale=$locale&op=do_nojs&id=1";

//"$site_url/install.php?profile=$profile&locale=$locale"
$url = "$site_url/install.php?profile=$profile&locale=$locale&op=finished&id=1";

$curl = new mycurl($url, true);
$curl->setPost($url_str);
$curl->createCurl();
echo $curl;


// set settings for loading database with site base settings
$url_str = "site_name=$site_name&";
$url_str.= "site_mail=$site_mail&";
$url_str.= "account[name]=$account_name&";
$url_str.= "account[mail]=$account_mail&";
$url_str.= "account[pass][pass1]=$account_pass&";
$url_str.= "account[pass][pass2]=$account_pass&";
$url_str.= "date_default_timezone=$date_default_timezone&";
$url_str.= "clean_url=$clean_url&";
$url_str.= "form_id=install_configure_form&";
$url_str.= "update_status_module[1]=$update_status_module";


$url = "$site_url/install.php?profile=default&locale=en";
$curl = new mycurl($url, true);
$curl->setPost($url_str);
$curl->createCurl();
echo $curl;

die();


