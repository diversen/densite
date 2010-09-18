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
       'site' => dt('The drupal base site to install. Use e.g. www.yoursite.net.'),
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
  
  // densite_load_db
  // keep trying to load the database until user rejects with a no,
  // or when user has loaded a database
  while  ( !$db_res = densite_load_db($site, $db_spec, 1)){ }

  // check to see if user wants a default cron line
  densite_enable_cron($site);
  densite_print_status($site, $db_res);
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
  densite_exec("mkdir " .   $settings['www_base_site']);  
  densite_exec("mkdir " .   $settings['www_base_htdocs']);  
  densite_exec("sudo cp -Rfp " .  $settings['drupal_source_path'] . "/* " .  $settings['www_base_htdocs']);  
  
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
  } else {
    densite_exec ("mkdir " . $settings['files_dir']);
  }
  
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
  return 0;
} 

// }}}
