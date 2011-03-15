<?php
// $Id: densitein.drush.inc,v 1.8 2009/06/25 13:55:48 deniver Exp $ 
// {{{ prepends 

/**
 * @file
 *   druensite.drush.inc
 */

/**
 * 
 */
// include configuration file
include_once "densite.conf";

// }}} 
// {{{ densitein_drush_command()

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and 
 * description.
 *
 * Notice how this structure closely resembles how 
 * you define menu hooks.
 * 
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function densitein_drush_command() {
  $items = array();

  $options['-f'] = 'Force. Do not halt on error';
  $options['-s'] = 'Silence. Less output';
  $options['--simulate'] = 'Use simulate when your want to se what will happen to your system. It will print the commands to be executed.';

  // the key in the $items array is the name of the command.
  $items['densitein'] = array(
    // the name of the function implementing your command.
    'callback' => 'densitein_densitein',
    // a short description of your command
    'description' => dt('Will download and install a version of drupal.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
    'arguments' => array(
       'site' => dt('The drupal base site to install. Use e.g. www.example.com'),
   ),
    'options' => $options

  );

  return $items;
}

// }}}
// {{{ densitein_drush_help($section)

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function densitein_drush_help($section) {
  switch ($section) {
    case 'drush:densitein':
        return dt("drush densitein is a script that downloads and installs a version of drupal on your server.");
  }
}

// }}}
// {{{ densitein_densitein($site)

/**
 * Install a downloaded version of drupal
 *   Try to create log files in the correct directory
 *   Try to copy settings to correct location
 *    
 * @param 
 *   string $site the name of the site to enable
 * @return 
 *   int 0 on succes 1 on failure
 *
 */
function densitein_densitein($site = '') {
  
  $res = densite_init();
  if (!$res){
    return $res;
  }

  if (empty($site)){
    densite_print(dt("You need to specify a site to install. "));
    $site = densite_readline(dt("Enter site name and hit return: "));
  }

  $res = densite_exec("sudo echo -e " . dt("Started enabling a web install"));
  if ($res) {
    densite_print(dt("Could not sudo. Exit"));
    return 1;
  }
  
  $settings = densitesa_get_filenames($site, $_SERVER['PWD']);
  
  $res = densitesa_download_drupal_file(&$settings);
  if (!$res){
    densite_print(dt("Could not download drupal") . "\n");
    return 1;
  }

  densite_print("\n");
  $db_spec = densite_enable_db ($site, 1);
  if (!$db_spec) { 
    densite_print(dt("Could not make database. Exit\n"));
    return 1;
  }

  $res = densitein_create_site($site, $db_spec, $settings);

  densitesa_create_virtualhost($site, $settings);
    
  // check to see if user wants a default cron line
  densite_enable_cron($site);
  // change file settings
  densite_chmod_settings($settings);

  if ($settings['drupal_major_version'] == '6'){
    // can not curl drupal 7 (requires javascript)
    // densite_curl_7 ($site, $db_spec, $settings);
    $readline_str = dt("Do you want to curl install first user with settings from densite.conf [Y/n]? ");
    $y_or_n = strtoupper(densite_readline($readline_str));
    if ($y_or_n == 'Y' || empty($y_or_n)){
      densite_curl_6 ($site, $db_spec, $settings);
      densite_print_status($site, 1);
    }
  } else {
    densite_print_status($site, 0);
  }
  return 0;
}


// }}}
// {{{ densitein_create_site

/**
 * 
 * @param 
 *   string $site the drupal base website to enable
 * @param 
 *   array $db_spec an array holding the db_spec for rewriting the $db_url in settings.php
 * @return
 *   0 on success, positive int on failure
 */
function densitein_create_site($site, $db_spec, $settings){
  // first we attempt to make the base for the new site
  // then we make the htdocs dir 
  // we then copy the version in mind to the new htdocs base
  if (!file_exists($settings['www_base_site'])){
    densite_exec("mkdir " .   $settings['www_base_site']);
  }

  if (!file_exists($settings['www_base_htdocs'])){
    densite_exec("mkdir " .   $settings['www_base_htdocs']);
  }
  densite_exec("sudo cp -Rfp " .  $settings['drupal_source_path'] . "/* " .  $settings['www_base_htdocs']);
  densite_exec("sudo cp " . $settings['drupal_source_path'] . "/.htaccess " . $settings['www_base_htdocs']);
  
  if (DENSITE_REMOVE_TXT){
    densite_exec("sudo rm -Rf " .  $settings['www_base_htdocs'] . "/*.txt");  
  }
  
  // when we have used the downloaded file
  // delete it
  densite_exec("rm -Rf " . $settings['drupal_download_file']);
  densite_exec("cp " .      $settings['settings_file_default'] . " " .             $settings['settings_file']);
  
  // for drupal 5 files folder are placed in root of install
  // remove this folder for the new site 

  if ($settings['drupal_major_version'] == '5'){
    $settings['files_dir'] = $settings['www_base_htdocs'] . "/files";  
    densite_exec("sudo rm -rf " .   $settings['files_dir']);  
    densite_exec("mkdir " .   $settings['files_dir']);
    densite_exec ("sudo chmod 777 " . $settings['files_dir']);
  } 
 
  
  // search and replace $db_url
  // the same procedure will be used when on drupal-5.x or drupal-6.x 
  // the procedure is different for drupal-7.x version. They use an 
  // array for holding the database info 
  
  if ($settings['drupal_major_version'] == '7'){

    $settings['files_dir'] = $settings['www_base_htdocs'] . "/sites/default/files";
    densite_exec("sudo rm -rf " .   $settings['files_dir']);
    densite_exec("mkdir " .   $settings['files_dir']);
    densite_exec ("sudo chmod 777 " . $settings['files_dir']);

    $settings_str = densite_get_file($settings['settings_file_default']);
    $search = "\$databases = array();";
    $replace = "\$databases['default']['default'] = array(
      'driver' => '$db_spec[driver]',
      'database' => '$db_spec[database]',
      'username' => '$db_spec[username]',
      'password' => '$db_spec[password]',
      'host' => '$db_spec[host]',
    );";

    $settings_str = str_replace($search, $replace, $settings_str);
    // version 6.x
  } else {
    $settings['files_dir'] = $settings['www_base_htdocs'] . "/sites/default/files";
    densite_exec("sudo rm -rf " .   $settings['files_dir']);
    densite_exec("mkdir " .   $settings['files_dir']);
    densite_exec ("sudo chmod 777 " . $settings['files_dir']);
  
    $settings_str = densite_get_file($settings['settings_file_default']);
    $search = "\$db_url = 'mysql://username:password@localhost/databasename'";
    $replace = "\$db_url = " . "'$db_spec[driver]://$db_spec[username]:$db_spec[password]@$db_spec[host]/$db_spec[database]'";
    $settings_str = str_replace($search, $replace, $settings_str);
    densite_exec ("sudo chmod 777 " . $settings['files_dir']);
  }
  
  // write the new settings into settings.php
  // make the files dir 
  // set correct permissions for files dir and settings.php
  
  densite_write_settings($settings['settings_file'], $settings_str);
  return 0;
}

function densite_chmod_settings ($settings) {
  densite_exec ("sudo chmod " . DENSITE_FOLDER_PERMS . " " . $settings['files_dir']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['files_dir']);
  densite_exec ("sudo chmod " . DENSITE_SETTINGS_PERMS . " " . $settings['settings_file']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['settings_file']);

}

function densite_curl_6 ($site, $db_spec, $settings) {
    if (DENSITE_DEBUG){
        print_r($site);
        print_r($db_spec);
        print_r($settings);
    }

      // end of settings
     // create a url for curling db settings
    $url_str = "db_path=$db_spec[database]&";
    $url_str.= "db_user=$db_spec[username]&";
    $url_str.= "db_pass=$db_spec[password]&";
    $url_str.= "db_host=$db_spec[host]&";
    $url_str.= "db_prefix=&";
    $url_str.= "db_port=&";
    $url_str.= "op=Save and continue" . "&";
    $url_str.= "form_id=install_settings_form";
    $url_str = urlencode($url_str);

    $base_url = "http://$site/install.php";
    $url_1 = "$base_url?profile=" . DENSITE_PROFILE;

    $curl = new mycurl($url_1, true);
    $curl->createCurl();
    $end_str = $curl;

    $url_2 = $url_1 . "&locale=" . DENSITE_LOCALE;
    $curl = new mycurl($url_2, true);
    $curl->setPost($url_str);
    $curl->createCurl();
    $end_str.=$curl;

    $url_3 = $url_2 . "&op=do_nojs&id=1";

    $curl = new mycurl($url_3, true);
    $curl->setPost($url_str);
    $curl->createCurl();
    $end_str.=$curl;

    $url_4 = $url_2 . "&op=finished&id=1";

    $curl = new mycurl($url_4, true);
    $curl->setPost($url_str);
    $curl->createCurl();
    $end_str.=$curl;

    // set settings for loading database with site base settings
    $url_str = "site_name=" . DENSITE_SITE_NAME . "&";
    $url_str.= "site_mail=" . DENSITE_SITE_MAIL . "&";
    $url_str.= "account[name]=" . DENSITE_ACCOUNT_NAME . "&";
    $url_str.= "account[mail]=". DENSITE_ACCOUNT_MAIL . "&";
    $url_str.= "account[pass][pass1]=" . DENSITE_ACCOUNT_PASS . "&";
    $url_str.= "account[pass][pass2]=" . DENSITE_ACCOUNT_PASS . "&";
    $url_str.= "date_default_timezone=" . DENSITE_DATE_DEFAULT_TIMEZONE . "&";
    $url_str.= "clean_url=" . DENSITE_CLEAN_URL . "&";
    $url_str.= "form_id=install_configure_form&";
    $url_str.= "update_status_module[1]=". DENSITE_UPDATE_STATUS_MODULE;

    // use url_2 again
    //$url = "$base_url?profile=default&locale=en";
    $curl = new mycurl($url_2, true);
    $curl->setPost($url_str);
    $curl->createCurl();
    $end_str.=$curl;
    file_put_contents('./curl.html', $end_str);
    return 0;
}


class mycurl {
     protected $_useragent = 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1';
     protected $_url;
     protected $_followlocation;
     protected $_timeout;
     protected $_maxRedirects;
     protected $_cookieFileLocation = './cookie.txt';
     protected $_post;
     protected $_postFields;
     protected $_referer ="http://www.google.com";

     protected $_session;
     protected $_webpage;
     protected $_includeHeader;
     protected $_noBody;
     protected $_status;
     protected $_binaryTransfer;
     public    $authentication = 0;
     public    $auth_name      = '';
     public    $auth_pass      = '';

     public function useAuth($use){
       $this->authentication = 0;
       if($use == true) $this->authentication = 1;
     }

     public function setName($name){
       $this->auth_name = $name;
     }
     public function setPass($pass){
       $this->auth_pass = $pass;
     }

     public function __construct($url,$followlocation = true,$timeOut = 30,$maxRedirecs = 4,$binaryTransfer = false,$includeHeader = true,$noBody = false)
     {
         $this->_url = $url;
         $this->_followlocation = $followlocation;
         $this->_timeout = $timeOut;
         $this->_maxRedirects = $maxRedirecs;
         $this->_noBody = $noBody;
         $this->_includeHeader = $includeHeader;
         $this->_binaryTransfer = $binaryTransfer;

         $this->_cookieFileLocation = dirname(__FILE__).'/cookie.txt';

     }

     public function setReferer($referer){
       $this->_referer = $referer;
     }

     public function setCookiFileLocation($path)
     {
         $this->_cookieFileLocation = $path;
     }

     public function setPost ($postFields)
     {
        $this->_post = true;
        $this->_postFields = $postFields;
     }

     public function setUserAgent($userAgent)
     {
         $this->_useragent = $userAgent;
     }

     public function createCurl($url = 'nul')
     {
        if($url != 'nul'){
          $this->_url = $url;
        }

         $s = curl_init();

         curl_setopt($s,CURLOPT_URL,$this->_url);
         curl_setopt($s,CURLOPT_HTTPHEADER,array('Expect:'));
         curl_setopt($s,CURLOPT_TIMEOUT,$this->_timeout);
         curl_setopt($s,CURLOPT_MAXREDIRS,$this->_maxRedirects);
         curl_setopt($s,CURLOPT_RETURNTRANSFER,true);
         curl_setopt($s,CURLOPT_FOLLOWLOCATION,$this->_followlocation);
         curl_setopt($s,CURLOPT_COOKIEJAR,$this->_cookieFileLocation);
         curl_setopt($s,CURLOPT_COOKIEFILE,$this->_cookieFileLocation);

         if($this->authentication == 1){
           curl_setopt($s, CURLOPT_USERPWD, $this->auth_name.':'.$this->auth_pass);
         }
         if($this->_post)
         {
             curl_setopt($s,CURLOPT_POST,true);
             curl_setopt($s,CURLOPT_POSTFIELDS,$this->_postFields);

         }

         if($this->_includeHeader)
         {
               curl_setopt($s,CURLOPT_HEADER,true);
         }

         if($this->_noBody)
         {
             curl_setopt($s,CURLOPT_NOBODY,true);
         }
         /*
         if($this->_binary)
         {
             curl_setopt($s,CURLOPT_BINARYTRANSFER,true);
         }
         */
         curl_setopt($s,CURLOPT_USERAGENT,$this->_useragent);
         curl_setopt($s,CURLOPT_REFERER,$this->_referer);

         $this->_webpage = curl_exec($s);
         $this->_status = curl_getinfo($s,CURLINFO_HTTP_CODE);
         curl_close($s);

     }

   public function getHttpStatus()
   {
       return $this->_status;
   }

   public function __tostring(){
      return $this->_webpage;
   }
}


