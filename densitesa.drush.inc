<?php
// $Id: densitesa.drush.inc,v 1.7 2009/06/25 13:55:48 deniver Exp $ 

// {{{ prepends 

/**
 * @file
 *   druensitesa.drush.inc
 */

// }}} 
// {{{ densitesa_drush_command()

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
function densitesa_drush_command() {
  $items = array();

  $options['-f'] = 'Force. Do not halt on error';
  $options['-s'] = 'Silence. Less output';
  $options['--simulate'] = 'Use simulate when your want to se what will happen to your system. It will print the commands to be executed.';

  // the key in the $items array is the name of the command.
  $items['densitesa'] = array(
    // the name of the function implementing your command.
    'callback' => 'densitesa_densitesa',
    // a short description of your command
    'description' => dt('Enable a drupal base site. The new install will use the current codebase or download one.'),
    'arguments' => array(
       'site' => dt('The drupal site to enable (e.g. www.yoursite.net).'),
   ),
    'options' => $options
  );
  
  $items['dissitesa'] = array(
    // the name of the function implementing your command.
    'callback' => 'densitesa_dissitesa',
    // a short description of your command
    'description' => dt("Disables a specified drupal base site."),
    'arguments' => array(
       'site' => dt('The drupal base site to disable (e.g. yoursite.net).'),
   ),
    'options' => $options,
  );
  

  return $items;
}

// }}}
// {{{ densitesa_drush_help($section)

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
function densitesa_drush_help($section) {
  switch ($section) {
    case 'drush:dissitesa':
    case 'drush:densitesa':
        return dt("drush densitesa is a script that enables a specified site which can serve as the base site for other sites (multisite install). The command creates a database based on the default sites install profile of the current working drupal install. You can chose to copy this site or download a version of your choice. You will be prompted for the database install. Based on the info for the database install the script creates your basesite and folders for log files, and writes a new sites/default/settings.php with the database information. When moving to the newly created site you can use the densite command for creating new sub sites. The densitesa - like the densite command - command also creates a <VirutalHost> configuration and enables this by calling the a2ensite wrappers. Likewise, dissitesa remove a drupal base site by removing the www/yoursite.org folder and also the database and the virtual host. It then reloads apache2");
  }
}

// }}}
// {{{ densitesa_densitesa($site)

/**
 * Enable a drupal shared virtual host from a shared codebase
 *    
 * @param 
 *   string $site the name of the site to enable
 * @return 
 *   int 0 on succes 1 on failure
 */
function densitesa_densitesa($site) {
  $res = densite_init();
  if (!$res){
    return $res;
  }
  
  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');
  
  if (!$drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT'))
    return dt("Could not find a valid install of drupal") . "\n";

  if (empty($site)){
    densite_print(dt("You need to specify a site to create. "));
    $site = densite_readline(dt("Enter site name and hit return: "));
  }
  
  if (densite_base_exists($site) && empty($force)){
    densite_print(dt("That base site already exists.") . "\n");
    return 1;
  }
  
  densite_exec('sudo echo -e "Started enabling a basesite\n"');
  
  // enable a database
  $db_spec = densite_enable_db ($site);
  if (!$db_spec) {
    densite_print(dt("Could not make database. Exit\n"));
    return 1;
  }

  $res = densitesa_create_site($site, $db_spec);
  if (!$res) {
    densite_print(dt("Could not make site. Exit\n"));
    return 1;
  }
  densitesa_create_virtualhost($site);
  
  // densite_load_db
  // keep trying to load the database until user rejects with a no,
  // or when user has loaded a database
  while  ( !$db_res = densite_load_db($site, $db_spec)){ }

  // check to see if user wants a default cron line
  densite_enable_cron($site);
  densite_print_status($site, $db_res);
  return 0;
}

// }}}
// {{{ densitesa_create_site

/**
 * 
 * @param 
 *   string $site the drupal base website to enable
 * @param 
 *   array $db_spec an array holding the db_spec for rewriting the $db_url in settings.php
 * @return
 *   void on success, positive int on failure
 */
function densitesa_create_site($site, $db_spec){
  $settings = densitesa_get_filenames($site); 
  densite_print(dt("Which standalone site to you want to create:") . "\n");
  $readline_str = dt("You have these options (c)lone current base or (d)ownload a version:");
  $install_type = densite_readline($readline_str);
   
  switch ($install_type){
    case('d'):

      $res = densitesa_download_drupal_file($settings);  
      if (!$res) return 0;

      break;
    case('c'): 
      // clone the site where you are standing
      // set version path to current directory
      $settings['drupal_major_version'] = $m = substr(drush_drupal_version(), 0, 1); 
      $settings['settings_file_default'] = $settings['settings_file_default'] . "-" . $m . ".x";
      $settings['drupal_source_path'] = $settings['drupal_root'];
    break;
  
    default:
      densite_print(dt("Unknown option"));
      return 0;
      break;
  }
  // first we attempt to make the base for the new site
  // then we make the htdocs dir 
  // we then copy the version in mind to the new htdocs base
  // also we remove all .txt files so noone can see the drupal version we us

  densite_exec("mkdir " .   $settings['www_base_site']);  
  densite_exec("mkdir " .   $settings['www_base_htdocs']);  
  densite_exec("sudo cp -Rfp " .  $settings['drupal_source_path'] . "/* " .  $settings['www_base_htdocs']);  
  
  if (DENSITE_REMOVE_TXT){
    densite_exec("sudo rm -Rf " .  $settings['www_base_htdocs'] . "/*.txt");  
  }
  
  // when we have used the downloaded file
  // delete it


  
  // when cloning a site we only want to create the default site in sites/default
  // remove any other sites folder in the new location
  
  densite_exec("sudo rm -Rf " . $settings['sites_dir']);  
  densite_exec("mkdir " .   $settings['sites_dir']);  
  densite_exec("mkdir " .   $settings['sites_dir'] . "/default");  
  densite_exec("cp " .      $settings['settings_file_default'] . " " .             $settings['settings_file']);

  // remember to make a copy of all
  $all_dir = $settings['drupal_source_path'] . "/sites/all";
  densite_exec("sudo cp -Rfp " .  $all_dir . " " .  $settings['sites_dir']);  
  
  if ($install_type == 'd'){
    densite_exec("rm -Rf " . $settings['drupal_download_file']);
  }
  
  // for drupal 5 files folder are placed in root of install
  // remove this folder for the new site 

  if ($settings['drupal_major_version'] == '5'){
    $settings['files_dir'] = $settings['www_base_htdocs'] . "/files";  
    densite_exec("sudo rm -rf " .   $settings['files_dir']);  
    densite_exec("mkdir " .   $settings['files_dir']);  
  } else {
    densite_exec ("mkdir " . $settings['files_dir']);
  }
  
  // create the settinge.php from the default.settings.php
  // if cloning we will remove any files from the old files directory 

  // search and replace $db_url
  // the same procedure will be used when on drupal-5.x or drupal-6.x 
  // the procedure is different for drupal-7.x version. They use an 
  // array for holding the database info 
  
  if ($settings['drupal_major_version'] == '7'){
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
  } else {
  
    $settings_str = densite_get_file($settings['settings_file_default']);
    $search = "\$db_url = 'mysql://username:password@localhost/databasename'";
    $replace = "\$db_url = " . "'$db_spec[driver]://$db_spec[username]:$db_spec[password]@$db_spec[host]/$db_spec[database]'";
    $settings_str = str_replace($search, $replace, $settings_str);
  }
  
  // write the new settings into settings.php
  // make the files dir 
  // set correct permissions for files dir and settings.php
  
  densite_write_settings($settings['settings_file'], $settings_str);
  densite_exec ("sudo chmod " . DENSITE_FOLDER_PERMS . " " . $settings['files_dir']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['files_dir']);
  densite_exec ("sudo chmod " . DENSITE_SETTINGS_PERMS . " " . $settings['settings_file']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['settings_file']);
  return 1;
} 

// }}}
// {{{ densitesa_get_filenames($site)

/**
 * function for creating an array of the most used names when configuring 
 * the drupal site and the apache server
 * 
 * @param
 *   string   $site
 * @param
 *   string   $www_base when this is set it will be used instead of creating one from drupal_root
 * @return
 *   array    $filenames  an array of filenames to use when creating the site
 */
function densitesa_get_filenames($site, $www_base){

  if (empty($www_base)){
    $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
    $www_base = densite_get_sites_root($drupal_root);
  } else {
    $drupal_root = $_SERVER['PWD'];
  }
  $path = explode('/', __FILE__); array_pop($path); 
  $apache_default_conf = implode ('/', $path) . "/apache2/" . DENSITE_APACHE_FILE;
  $settings_file_default = implode ('/', $path) . "/settings/default.settings.php";

  $filenames = array(
    'drupal_root'            => $drupal_root,
    'settings_file'          => $www_base . "/" . $site . "/". DENSITE_APACHE_HTDOCS . "/sites/default/settings.php",
    'settings_dir'           => $www_base . "/" . $site . "/" . DENSITE_APACHE_HTDOCS . "/sites/default",
    'sites_dir'              => $www_base . "/" . $site . "/" . DENSITE_APACHE_HTDOCS . "/sites",
    'settings_file_default'  => $settings_file_default,
    'files_dir'              => $www_base . "/" . $site . "/" . DENSITE_APACHE_HTDOCS . "/sites/default/files",
    'www_base'               => $www_base,
    'www_base_site'          => $www_base . "/" . $site,
    'www_base_htdocs'        => $www_base . "/" . $site . "/". DENSITE_APACHE_HTDOCS,
    'www_base_site_logs'     => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS,
    'www_base_site_access'   => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS . "/access.log",
    'www_base_site_error'    => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS . "/error.log",
    'apache_default_conf'    => $apache_default_conf,
    'apache_sites_avail'     => "/etc/apache2/sites-available/" . $site,
  );
  return $filenames;
}

// }}}
// {{{ densitesa_create_virtual_host ($site)

/** 
 * function for making an apache2 virtualhost, and log files
 *
 * @param
 *   string $site the site to enable
 * @return
 *   void all error handling is being done in densite_exec
 */
function densitesa_create_virtualhost($site, $settings = ''){
  if (empty($settings)){
    $settings = densitesa_get_filenames($site); 
  }
  
  // attempt to create the log files
  if (!file_exists($settings['www_base_site_logs'])){
    densite_exec("mkdir " . $settings['www_base_site_logs']);
  }
  densite_exec("touch " . $settings['www_base_site_access']);
  densite_exec("touch " . $settings['www_base_site_error']);

  // use /tmp to create a copy of the file
  densite_exec("cp " . $settings['apache_default_conf'] . " " .  "/tmp/apache2.conf");
  $apache_settings_str = densite_get_file("/tmp/apache2.conf");
  $search = array("server_name", "document_root", "site_root", "allow_ip");
  
  // create correct settings for the new virtualhost file
  $replace = array($site, $settings['www_base_htdocs'], $settings['www_base_site'], DENSITE_ALLOW_IP);
  $apache_settings_str = str_replace($search, $replace, $apache_settings_str);
  densite_write_settings("/tmp/apache2.conf", $apache_settings_str);

  densite_exec ("sudo cp /tmp/apache2.conf " . $settings['apache_sites_avail']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['settings_file']);
  
  densite_update_etc_hosts ($site);


  densite_exec ("sudo a2ensite $site");
  densite_exec ("sudo /etc/init.d/apache2 reload");
  return 0;
}

// }}}
// {{{ densitesa_dissitesa($site)

/**
 * Disable a drupal website
 *
 * @param   string $site the drupal website to disable
 * @return  int 0 on success and 1 on failure
 */
function densitesa_dissitesa($site) {
  $res = densite_init();
  if (!$res){
    return $res;
  }
  
  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');
  
  $all_sites = array();
  $all_sites = densite_get_all_sites();
  
  if (empty($site)){
    if ( empty($all_sites) && empty($force) ){
      densite_print(dt("There does not seem to be any valid sites installed in sites. Aborting!") . "\n");
      return 1;
    } else {
      densite_print(dt("You need to specify a base site to delete. Only base sites that does not serve any sub sites can be deleted: \n"));
      densite_list_all_sites();
      $site = densite_readline(dt("Enter base site to disable: "));
    }
  }
  
  if (!densite_base_exists($site) && empty($force) ){
    densite_print(dt("That site does not seem to exist. Aborting!") . "\n");
    return 1;
  }
  
  // prevent deleting sites that has subs
  $sub_sites = densite_get_sub_sites($site);
  if(!empty($sub_sites)){
    densite_print(dt("The site you are trying to delete has sub sites: You need to delete those first!\n"));
    return 1;
  }
  
  densite_exec('sudo echo -e "Starting disabling a basesite\n"');
  
  $settings = densitesa_get_filenames($site);

  $readline_str = dt("Delete the following website: ") . $site . dt(" [Y/n]? ");
  $site_delete_confirm = strtoupper(densite_readline($readline_str)); 
  if ($site_delete_confirm == 'Y' || empty($site_delete_confirm)){
    
    // delete database
    $result = densite_disable_db($settings['settings_file']);
    
    $readline_str = dt("Remove apache2 configuration and restart apache: ") . $site . dt(" [Y/n]? ");
    $apache2_delete_confirm = strtoupper(densite_readline($readline_str)); 
    
    if ($apache2_delete_confirm == 'Y' || empty($apache2_delete_confirm)){
      densite_exec("sudo a2dissite " .                $site);
      densite_exec("sudo /etc/init.d/apache2 reload");
      densite_exec("sudo rm " .                       $settings['apache_sites_avail']);
    }
    
    $readline_str = dt("Remove all site files found in ") . $settings['www_base_site'] . dt(" [Y/n]? ");
    $site_files_delete_confirm = strtoupper(densite_readline($readline_str)); 
    
    if ($site_files_delete_confirm == 'Y' || empty($site_delete_confirm)){
      densite_exec("sudo rm -rf " .                   $settings['www_base_site']);
    }

    // remove site from crontab
    densite_print(dt("Disabling site from crontab") . "\n");
    densite_disable_cron($site);
  }
  return 0; //OK
}

// }}}
// {{{ densitesa_download_drupal_file (&$settings)
/**
 * function for downloading a version of drupal
 *
 * @param 
 *   &$settings the settings is passed in order to change it as we download the file
 * @return
 *   int 1 on success amd 0 on failure
 */

function densitesa_download_drupal_file(&$settings){
  
  $readline_str = dt("Enter version e.g. 5.18 or 6.12 (exact) or 5.x-dev or 6.x-dev (dev versions): ");
  $version = densite_readline($readline_str);
  $settings['drupal_download_file'] = "drupal-" . $version;
  $settings['drupal_source_path'] = $settings['drupal_root'] . "/" . $settings['drupal_download_file'];
  
  // start out by checking if the file exists on the download server
  // notice that we force the execution of the script 
  // because we are only checking if file exists
  $command = "wget --spider " . DENSITE_DOWNLOAD_URL;
  $exists = 0;
  system ($command . $settings['drupal_download_file'] . ".tar.gz", $exists);

  // ok
  if (!$exists){
    $settings['drupal_major_version'] = $m = substr($version, 0, 1);
    $settings['settings_file_default'] = $settings['settings_file_default'] . "-" . $m .".x";

    // resume download use downloaded file
    $command = "wget -c " . DENSITE_DOWNLOAD_URL;

    // we use system here to see the process of wget
    system ($command . $settings['drupal_download_file'] . ".tar.gz" , $res);
    densite_exec ("tar xfz " . $settings['drupal_download_file'] . ".tar.gz");

    // don't remove.
    //densite_exec ("rm " . $settings['drupal_download_file'] . ".tar.gz");
    return 1;
  } else {
    densite_print(dt("File does not exist on remote server") . "\n");
    return 0;
  }
}

// }}}
