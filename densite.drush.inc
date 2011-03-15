<?php
// $Id: densite.drush.inc,v 1.17 2009/09/03 09:57:32 deniver Exp $ 
// {{{ prepend

/**
 * @file
 *   druensite.drush.inc
 * 
 * functions connected with commands that are used from the shell
 * will return 0 on succes and positive int on failure. 
 * 
 * functions that are used internally will do the opposite.
 */

// include configuration file
$densite_conf = dirname(__FILE__) . "/densite.conf";
if (file_exists($densite_conf)){
  require_once $densite_conf;
} else { 
  $densite_conf_dist = dirname(__FILE__) . "/dist-densite.conf";
  require_once $densite_conf_dist;
}
// }}}
// {{{ densite_init()

/**
 *
 * function for checking current configuration
 */
function densite_init(){
  $densite_dir = dirname(__FILE__);
  $densite_conf = dirname(__FILE__) . "/densite.conf";
  $densite_conf_dist = dirname(__FILE__) . "/dist-densite.conf";
  
  $densite_apache2_dir = dirname(__FILE__);
  $densite_apache2_conf = dirname(__FILE__) . "/apache2/apache2.conf";
  $densite_apache2_conf_dist = dirname(__FILE__) . "/apache2/dist-apache2.conf";
  
  $densite_sql_dir = dirname(__FILE__) . "/sql";
  
  if ( file_exists($densite_conf) && file_exists($densite_apache2_conf) && file_exists($densite_sql_dir) ){
    return 1;
  }

  if (!file_exists($densite_conf)){
    $output = dt("Configuration file not found: ") . $densite_conf . "\n";
    densite_print($output);
    $readline_str = dt("Do you want to create: ") . $densite_conf; 
    $readline_str.= dt(" from ") . $densite_conf_dist . dt(" [Y/n]?"); 
    $create_densite_conf = strtoupper(densite_readline($readline_str));
    if ($create_densite_conf == 'Y' || empty($create_densite_conf)){
      densite_exec("cp " . $densite_conf_dist . " " . $densite_conf);
    } else {
      densite_print(dt("We can not proceed without a default configuration file. Exit" . "\n"));
      return 0;
    }
  }
  
  if (!file_exists($densite_apache2_conf)){
    $output = dt("Apache2 configuration file not found: ") . $densite_apache2_conf . "\n";
    densite_print($output);
    $readline_str = dt("Do you want to create: ") . $densite_apache2_conf; 
    $readline_str.= dt(" from ") . $densite_apache2_conf_dist . dt(" [Y/n]?"); 
    $create_apache2_conf = strtoupper(densite_readline($readline_str));
    if ($create_apache2_conf == 'Y' || empty($create_apache2_conf)){
      densite_exec("cp " . $densite_apache2_conf_dist . " " . $densite_apache2_conf);
    } else {
      densite_print(dt("We can not proceed without a default apache file. Exit" . "\n"));
      return 0;
    }
  }
  
  if (!file_exists($densite_sql_dir)){
    $output = dt("Sql dir not found: ") . $densite_sql_dir . "\n";
    densite_print($output);
    $readline_str = dt("Do you want to create: ") . $densite_sql_dir . dt(" [Y/n]?"); 
    $create_sql_dir = strtoupper(densite_readline($readline_str));
    if ($create_sql_dir == 'Y' || empty($create_sql_dir)){
      densite_exec("mkdir " . $densite_sql_dir);
    } else {
      densite_print(dt("We can not proceed without a sql dir. Exit" . "\n"));
      return 0;
    }
  }
  densite_print(dt("Look at the two configuration files created and change them as you like:") . "\n");
  densite_print($densite_conf . "\n");
  densite_print($densite_apache2_conf . "\n");
  return 1;
}

// }}}
// {{{ densite_drush_command()

/**
 * Implementation of hook_drush_command().
 * 
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function densite_drush_command() {
  $items = array();
  // the options we want to use in this script
  $options['-f'] = 'Force. Do not halt on error';
  $options['-s'] = 'Silence. Less output';
  $options['--simulate'] = 'Use simulate when you want to see what will happen to your system. It will print the commands to be executed.';

  // the key in the $items array is the name of the command.
  $items['densite'] = array(
    // the name of the function implementing your command.
    'callback' => 'densite_densite',
    // a short description of your command
    'description' => dt('Enable a drupal sub-site install. Will create the specified sub-site in the base /sites folder. E.g. www.yoursite.net will be placed in sites/www.yoursite.net'),
    'arguments' => array(
       'site' => dt('The drupal sub-site to enable. e.g. www.yoursite.net. Only works when a drupal install is enabled.'),
   ),
    'options' => $options
  );
  
  $items['dissite'] = array(
    // the name of the function implementing your command.
    'callback' => 'densite_dissite',
    // a short description of your command
    'description' => dt("Disables a drupal sub-site. Will remove the spcified sub-site. e.g. www.yoursite.net in sites/www.yoursite.net will be removed."),
    'arguments' => array(
       'site' => dt('The drupal site to disable. e.g. yoursite.net. Only works when a drupal install is enabled.'),
   ),
    'options' => $options, 
  );
  
  $items['listsites'] = array(
    // the name of the function implementing your command.
    'callback' => 'densite_list_all_sites',
    // a short description of your command
    'description' => dt("Lists all drupal base sites and all sub-sites of base sites enabled on your server."),
   );
  
  $items['densite_test'] = array(
    // the name of the function implementing your command.
    'callback' => 'densite_test',
    'description' => dt("Used for testing"),
    // a short description of your command
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
   );
  
  return $items;
}

// }}}
// {{{ densite_drush_help()

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
function densite_drush_help($section) {
  switch ($section) {
    case 'drush:dissite':
    case 'drush:densite':
        return dt("drush densite is a script that enables the specified site inside a drupal shared code base. The command creates a database based on the default sites install profile inside the sites directory (default) or you will be prompted for the new database install. Based on the info for the database install the script creates e.g. sites/www.yoursite.org and sites/yoursite.org/settings.php with the database scheme. It also creates a <VirutalHost> configuration and enables this by calling the a2ensite wrappers. Likewise, dissite disables a site by removing the folder inside sites/ e.g. sites/www.yoursite.net and also the database and the virtual host.");
    case 'drush:listsites':
        return dt("Lists all drupal base sites and all sub-sites to base sites enabled on your server.");
  }
}

// }}} densite_drush_help ()
// {{{ densite_exec()

/**
 * Any execution (exec) of a command is wrapped around the densite_exec command
 * Then we can easily control when to halt on errors or when to force.
 * We can also control if the exec command shall be used in silence or 
 * simulated.
 *
 * @param
 *   string $command the command to be executed
 * @param
 *   string $force set in case we want the script to continue executing. This
 *	 also silences the command. This param is meant as a way to test shell 
 *   commands, e.g. testing if a database exists or not, without killing the
 *   script. 
 * @return
 *   int $result, the $result of the executed shell command 0 on success and 
 *   positive int on failure. Depending on the $force flag it will stop 
 *   executing the script on failure (positive int) and return the result. 
 */
function densite_exec($command, $force = 0){
  // get options

  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');

  // if we set the force when using densite_exec
  // from inside these scripts 
  // we will only use the result internal
  if (!$force){
    $force = drush_get_option('f');
  } else {
    $silent = 1;
  }

  if ($simulate){
    $message = SIMULATE_PREPEND;
    $message.=$command . "\n";
    densite_print($message);
    return 0;
  }
  
  $output = $result = 0;
  // exec command in silence. Store output of executed command in $output
  exec($command.' 2>&1', $output, $result);

  // ERROR executing command
  if ($result){
    $message.= ERROR_PREPEND;
    $message.= $command . "\n";
    if (empty($silent)){
      densite_print($message);
      densite_print($output);
    }
    if (empty($force)){
      return exit(1);
    }
  }

  // OK executing command
  if (!$result){
    $message.= NO_ERROR_PREPEND;
    $message.= $command . "\n";
    
    if (empty($silent)){
      densite_print($message);
      densite_print($output);
    }
  }
  return $result; 
}

// }}} densite_exec()
// {{{ densite_print($output)

/**
 * function for writing output to the screen. Or making a nice string
 * out of an array
 * 
 * @param
 *    int $ret if a positive integer. The function will not print, but return
 *    content as a string instead
 * @param
 *   string $output the return string from densite_exec commands to write out
 * @return
 *   string $buf optional. If $ret is set function will return a string 
 */
function densite_print($output, $ret = 0){
  // exec uses a array to store output in
  $buf = '';
  if (is_array($output)){
    foreach ($output as $line) {
      if (!$ret){
        print($line) . "\n";
      } else {
        $buf.=$line . "\n";
      }
    }
  // else: just print the line
  } else {
    if (!$ret){
      print($output);
    }
  }  
  if ($ret) { 
    return $buf; 
  }
}

// }}}
// {{{ densite_densite($site)

/**
 * function which will try to enable a sub-site to a drupal base site
 *    
 * @param 
 *    string $site the name of the site to enable
 * @return 
 *    int 1 on failure on 0 on success. This will send exit code to
 *    the shell.       
 */
function densite_densite($site) {

  $res = densite_init();
  if (!$res){
    return $res;
  }

  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');

  //make sure we are running inside a installed version of drupal
  if (!$drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT'))
    return dt("Could not find a valid install of drupal") . "\n";

  if (empty($site)){
    densite_print(dt("You need to specify a site to create. "));
    $site = densite_readline(dt("Enter sub-site name and hit return: "));
  }
  
  if (empty($site) && empty($force)){
    densite_print(dt("You need to specify a site. Exit. ") . "\n");
    return 1;
  }
  
  if (densite_sub_exists($site) && empty ($force)){
    densite_print(dt("That site seems to exist as a sub-site already. Exit.") . "\n");
    return 1;
  }
  
  if (densite_base_exists($site) && empty ($force)){
    densite_print(dt("That site seems to exist as base site. Exit.") . "\n");
    return 1;
  }

  // sudo :         sudo to ensure that we can log in as su 
  // database:      enable a database or use existing
  // drupal:        create files for site/ and site/files
  // apache:        create apache log files and configuration and restart apache2
  // load database: ask to populate the database
  // cron:          enable cron job

  $res = densite_exec("sudo echo -e " . dt("Started enabling a sub-site"));
  if ($res) {
    densite_print(dt("Could not sudo. Exit"));
    return 1;
  }
  densite_print("\n");
  $db_spec = densite_enable_db ($site);
  if (!$db_spec) { 
    densite_print(dt("Could not make database. Exit\n"));
    return 1;
  }
  densite_create_site($site, $db_spec);
  densite_create_virtualhost($site);
  
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
// {{{ densite_print_status
/*
 * function for printing status of install
 * @param   int  $db_res
 */ 
function densite_print_status($site){
    $status = dt("Manual install. Continue on http://") . $site . "/install.php\n";
    $status.= dt("Or use:\n\ncd $site/htdocs && drush site-install\n\n to do it from command line\n");
    densite_print(dt($status));
    return 0;
}

// }}}
// {{{ densite_dissite($site)

/**
 * function (callback) Disable a drupal sub-site
 * returns to the shell 
 * 
 * @param   string $site the drupal website to disable
 * @return  int 0 on success and 1 on failure
 */
function densite_dissite($site) {
  $res = densite_init();
  if (!$res){
    return $res;
  }
  
  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');
  
  $all_sites = array();
  $all_sites = densite_get_sub_sites();
  
  if (empty($site)){
    if (empty($all_sites) && empty($force)){
      densite_print(dt("There do not seem to be any valid sub-sites installed in this base.") . "\n");
      return 1;
    } else {
      densite_print(dt("You need to specify a sub-site: Your choices are: \n"));
      densite_list_sub_sites();
      $site = densite_readline(dt("Enter site to disable: "));
    }
  }
  
  if (densite_base_exists($site) && empty($force)){
    densite_print(dt("That site is a base site. Use the command 'drush dissitesa $site' to remove it.") . "\n");
    return 1;
  }


  if (!densite_sub_exists($site) && empty($force)){
    densite_print(dt("That site does not seem to exist.") . "\n");
    return 1;
  }

  if (!densite_sub_to_current($site)){
    densite_print(dt("That site is not a sub site to a current drupal base. You need to go to the correct base site.") . "\n");
    return 1;
  }
  
  densite_exec('sudo echo -e "Started disabling a sub-site\n"');
  
  $settings = densite_get_filenames($site);
  $readline_str = dt("Delete the following website: ") . $site . dt(" [Y/n]? ");
  $site_delete_confirm = strtoupper(densite_readline($readline_str)); 
  if ($site_delete_confirm == 'Y' || empty($site_delete_confirm)){
    
    // delete database
    $result = densite_disable_db($settings['settings_file']);
    
    $readline_str = dt("Remove apache2 configuration and reload apache: ") . $site . dt(" [Y/n]? ");
    $apache2_delete_confirm = strtoupper(densite_readline($readline_str)); 
    
    if ($apache2_delete_confirm == 'Y' || empty($apache2_delete_confirm)){
      densite_exec("sudo a2dissite " .                $site);
      densite_exec("sudo /etc/init.d/apache2 reload");
      densite_exec("sudo rm " .                       $settings['apache_sites_avail']);
    }
    
    densite_print(dt("Remove all site files found in: ") . "\n");
    densite_print($settings['www_base_site'] . "\n" . $settings['settings_dir']);
    $readline_str = dt(" [Y/n]? ");
    $site_files_delete_confirm = strtoupper(densite_readline($readline_str)); 
    
    if ($site_files_delete_confirm == 'Y' || empty($site_delete_confirm)){
      densite_exec("sudo rm -rf " .                   $settings['www_base_site']);
      densite_exec("sudo rm -rf " .                   $settings['settings_dir']);
    }
    densite_print(dt("Disabling site from crontab") . "\n");
    densite_disable_cron($site);
  } 

  return 0;
}

// }}}
// {{{ densite_create_virtualhost($site)

/** 
 * we assume your websites are arranged like this: 
 * 
 * /home/user/www/default_drupal_website/htdocs
 * /home/user/www/default_drupal_website/logs
 * 
 * /home/user/www/new_drupal_website/logs
 * /home/user/www/new_drupal_website/logs/error.log
 * /home/user/www/new_drupal_website/logs/access.log
 *
 * Creates log files for a website
 *
 *
 * @param
 *   string $site the site to enable
 * @return
 *   void all error handling is being done in densite_exec
 */
function densite_create_virtualhost($site, $standalone = FALSE){
  $settings = densite_get_filenames($site);
    
  // create the log files
  densite_exec("mkdir " . $settings['www_base_site']);
  densite_exec("mkdir " . $settings['www_base_site_logs']);
  densite_exec("touch " . $settings['www_base_site_access']);
  densite_exec("touch " . $settings['www_base_site_error']);
  
  // search and replace in apache configuration
  densite_exec("cp " . $settings['apache_default_conf'] . " /tmp/apache2.conf");
  $apache_default_string = densite_get_file("/tmp/apache2.conf");
  $search = array("server_name", "document_root", "site_root", "allow_ip");
  $replace = array($site, $settings['drupal_root'], $settings['www_base_site'], DENSITE_ALLOW_IP);
  $apache_default_string = str_replace($search, $replace, $apache_default_string);

  // write the file 
  densite_write_settings("/tmp/apache2.conf", $apache_default_string);
  densite_exec ("sudo cp /tmp/apache2.conf " . $settings['apache_sites_avail']);

  // reload apache
  densite_exec ("sudo a2ensite $site");
  densite_exec ("sudo /etc/init.d/apache2 reload");
  densite_update_etc_hosts ($site);

}

// }}}
// {{{ densite_create_site($site, $db_spec)

/**
 * function for creating new drupal site folder in e.g.:
 * sites/example.org
 *
 * @param 
 *   string $site the drupal website to enable
 * @param 
 *   array $db_spec an array holding the db_spec for rewriting the $db_url in settings.php
 * @return
 *   void all error handling is being done in densite_exec
 */

// FIX: Maybe check if directory is writable and return 1 or 0.
function densite_create_site($site, $db_spec){
  $settings = densite_get_filenames($site);
  
  // create settings dir 
  // cp settings file
  densite_exec("mkdir " . $settings['settings_dir']);
  densite_exec("cp " . $settings['settings_file_default'] . " " . $settings['settings_file']);
  
  // search and replace $db_url and write new settings file
  $settings_string = densite_get_file($settings['settings_file']);

  $search = "\$db_url = 'mysql://username:password@localhost/databasename'";
  $replace = "\$db_url = " . "'$db_spec[driver]://$db_spec[username]:$db_spec[password]@$db_spec[host]/$db_spec[database]'";
  $settings_string = str_replace($search, $replace, $settings_string);
  
  densite_write_settings($settings['settings_file'], $settings_string);
  
  // secure settings for files and settings
  densite_exec ("sudo mkdir " . $settings['files_dir']);
  densite_exec ("sudo chmod " . DENSITE_FOLDER_PERMS . " " . $settings['files_dir']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['files_dir']);
  densite_exec ("sudo chmod " . DENSITE_SETTINGS_PERMS . " " . $settings['settings_file']);
  densite_exec ("sudo chown " . DENSITE_DEFAULT_USER . ":" . DENSITE_DEFAULT_GROUP . " " . $settings['settings_file']);
} 

// }}}
// db and sql related functions 
// {{{ densite_create_db($db_spec)

/**
 * function for creating a database. supporting mysql and postgres
 *
 * @param 
 *   array $db_spec with information about the database to be created
 * @return 
 *   int  1 on success and 0 on failure. 
 *
 */
function densite_create_db($db_spec){
  if ($db_spec['driver'] == 'mysql' || $db_spec['driver'] == 'mysqli'){
    $command = "mysqladmin -h" .  $db_spec['host'] . " ";
    $command.= "-p" .             $db_spec['password'] . " ";
    $command.= "-u" .             $db_spec['username'] . " ";
    $command.= "create " .        $db_spec['database'];
    densite_exec($command);
  } else if ($db_spec['driver'] == 'pgsql'){
    $command = "export PGPASSWORD='$db_spec[password]'; ";
    $command.= "createdb -O " .   $db_spec['username'] . " "; 
    $command.= "-U " .            $db_spec['username'] . " "; 
    $command.="-h " .             $db_spec['host'] . " "; 
    if (!empty($db_spec['port'])){
      $command.= "-p " . $db_spec['port'] . " ";
    }
    $command.="-E " . DENSITE_SQL_ENCODING . " " . $db_spec['database'];
    densite_exec($command);
  } else {
    densite_print(dt("Unknown driver: ") . $db_spec['driver'] . "\n");
    return 0;
  }
  return 1;
}

// }}}
// {{{ densite_delete_db($db_spec)

/**
 * function for deleting a database
 * @param 
 *   array $db_spec with information about the database to be deleted
 * @return 
 *   int 1 on success 0 on failure
 */
function densite_delete_db($db_spec){
  if ($db_spec['driver'] == 'mysql' || $db_spec['driver'] == 'mysqli'){
    $command = "mysqladmin --force ";
    $command.= "-h" . $db_spec['host'] . " ";
    $command.= "-p" . $db_spec['password'] . " ";
    $command.= "-u" . $db_spec['username'] . " drop "; 
    $command.=        $db_spec['database'];
    densite_exec($command);
  } else if ($db_spec['driver'] == 'pgsql'){
    $command = "export PGPASSWORD='$db_spec[password]';";
    $command.= "dropdb ";
    $command.= "-h " . $db_spec['host'] .  " ";
    $command.= "-U " . $db_spec['username']  . " ";
    if (!empty($db_spec['port'])){
      $command.= "-p " . $db_spec['port'] . " ";
    }
    $command.= '"' . $db_spec['database']. '"';
    densite_exec($command);
  } else {
    densite_print(dt("Unknown driver: ") . $db_spec['driver']);
    return 0;
  }
  return 1;
}

// }}}
// {{{ densite_get_db_spec()

/**
 * Wrapper around the drush functions to get some db specs
 * At the time of writing these two functions returned different
 * stuff so i had to combine them. 
 * 
 * @return 
 *   array  $db_spec array holding the information about the database
 *          of the base site where we are working. 
 */
function densite_get_db_spec(){
  $db_spec = _drush_sql_get_db_spec();  
  // we need to use another version to get a correct driver 
  $db_spec_context = drush_get_context('DRUSH_DB_CREDENTIALS');
  $db_spec['driver'] = $db_spec_context['driver'];
  return $db_spec;
}

// }}}
// {{{ densite_enable_db($site)

/**
 * function for creating a database using readline commands
 * the function asks some questions and out of them the database 
 * will be created
 *
 * @param
 *   string $site the name of the website to enable a database for
 * @param
 *   int  $install. If positive we are in install mode and 0 we are
 *        installaing a sub or base site. 
 * @return 
 *   array $db_spec containing the newly created database info
 *         or 0 on failure
 */
function densite_enable_db ($site, $install = 0){
  $db_spec = array();
  $db_spec = densite_get_db_spec();  
  $db_name = ereg_replace ("[\.-]", "", $site);

  if ($install){
    // if it's an install only use manual procedure
    $db_spec = densite_enable_db_manual();
  } else {
    $output = dt("Found a default connection with:\n");
    $output.= dt("Driver:         ") . $db_spec['driver'] .   "\n";
    $output.= dt("Host:           ") . $db_spec['host'] .     "\n"; 
    $output.= dt("User:           ") . $db_spec['username'] . "\n";
    $output.= dt("Password:       ") . $db_spec['password'] . "\n\n";
    $output.= dt("Automatic generated name for database will be:") . "\n";
    $output.= dt("database        ") . $db_name . "\n\n";

    densite_print($output);

    $readline_str = dt("Press 'Y' to use these settings. Press 'n' to proceed with manual install of database");
    $readline_str.= dt(" [Y/n]? ");
    $db_use_default = strtoupper(densite_readline($readline_str));

    // proceeding manually
    if ( $db_use_default == 'Y' || empty($db_use_default) ){
      $db_spec['database'] = $db_name;
    } else {
      $db_spec = densite_enable_db_manual();
    }
  }

  densite_print (dt("Proceeding and trying to create database. \nYou may be prompted for password: "). "\n");
  
  // Check to see if this database already exists and if we can connect to it
  
  if (densite_database_exists($db_spec)){
    densite_print("\n" . dt("Database ") . $db_spec['database'] . dt(" already exists and user can connect to it.") . "\n");
    $readline_str = dt("Do you want to drop the given database and create it again"). dt(" [Y/n]? ");
    $db_use_exists = strtoupper(densite_readline($readline_str));

    // Check to see if the user wants to use existing database
    if ( $db_use_exists == 'Y' || empty ($db_use_exists) ){

      // delete database
      if (!densite_delete_db($db_spec)){
        densite_print(dt("Could not drop database. ") . $db_spec['database'] . "\n");
        return 0;
      }  
      // continue and create databse
      if(!densite_create_db($db_spec)){
        return 0;
      } else {
        return $db_spec;
      }
    }
  // database does not exist. Lets create it. 
  } else {
    if(!densite_create_db($db_spec)){
      densite_enable_db ($site, $install);
      return 0;
    } else {
      return $db_spec;
    }
  } 
  return 0;
}

// }}}
// {{{ densite_enable_db_manual()

/**
 * function for enabling a database manually.
 * @return 
 *   array $db_spec an array holding information about the database to be created
 */
function densite_enable_db_manual(){
    $settings = array();
    densite_print(dt("Manual install of the database. ") . "\n");
    if (!$db_spec['driver'] = DENSITE_DB_DRIVER) {
      densite_print(dt("You PHP cli version reports you have support for these drivers: "));
      if (extension_loaded('mysql')){
        densite_print(dt('mysql '));
      } 
      if (extension_loaded('mysqli')){
        densite_print(dt('mysqli '));
      }
      if (extension_loaded('pgsql')){
        densite_print(dt('pgsql '));
      }
      densite_print("\n");
      $db_spec['driver']    = densite_readLine(dt("Driver:         "));
    }
    else {
      densite_print("Driver from settings from densite.conf: " . $db_spec['driver'] . "\n");
    }
    if (!$db_spec['host'] = DENSITE_DB_HOST) {
      $db_spec['host']      = densite_readLine(dt("Hostname:       "));
    }
    else {
      densite_print("Host from densite.conf: " . $db_spec['host'] . "\n");
    }
    if (!$db_spec['username'] = DENSITE_DB_USERNAME) {
      $db_spec['username']  = densite_readLine(dt("Username:       "));
    }
    else {
      densite_print("Usernames from densite.conf: " . $db_spec['username'] . "\n");
    }
    if (!$db_spec['password'] = DENSITE_DB_PASSWORD) {
      $db_spec['password']  = densite_readLine(dt("Password:       "));
    }
    else {
      densite_print("Password from densite.conf: " . $db_spec['password'] . "\n");
    }
    $db_spec['database'] =  densite_readLine(dt("Database name:  ")); 
    return $db_spec;
}

// }}}
// {{{ densite_database_exists($db_spec)
/**
 * function for checking if database exists.
 * @param 
 *   array $db_spec an array with the information about the database
 * return
 *   int  1 if exists 0 if not
 */
function densite_database_exists($db_spec){
  // the command returns with positive int if mysql can not use 
  // the database specified in $db_spec[database]
  if ($db_spec['driver'] == 'mysql' || $db_spec['driver'] == 'mysqli'){
    $res = densite_exec("echo 'use $db_spec[database]' | mysql -u$db_spec[username] -p$db_spec[password] -h$db_spec[host]", 1);
    if ($res){
      return 0;
    } else {
      return 1;
    }
  } else if ($db_spec['driver'] == 'pgsql') {
    // this shell command returns 1 if a pgsql database is found with the name $db_spec['database'];
    $command = "export PGPASSWORD='$db_spec[password]';";
    $command.= "sql=\"select count(1) from pg_catalog.pg_database where datname = '$db_spec[database]'\" && ";
    $command.= "cmd=\"psql -U$db_spec[username] -h$db_spec[host] -t -c \\\"\$sql\\\"\""; 
    $command.= " && db_exists=`eval \$cmd` && exit \$db_exists";
    
    // FIX: NICER WAYS TO DO THIS
    // print $command . "\n";
    $res = densite_exec($command, 1); 
    // print "res $res";
    if ($res){
      return 1;
    } else {
      return 0;
    }
  }
}

// }}}
// {{{ densite_disable_db()

/**
 * function for showing a confirm form on deleting a database
 * 
 * @param
 *   string $settings_file the settings.php where the database will be deleted
 * @return 
 *   int 0 on success postive int on failure
 */
function densite_disable_db ($settings_file = NULL){

  $db_spec = _drush_sql_get_spec_from_settings($settings_file, 'source');
  $readline_str = dt("Delete the following database: ") . $db_spec['database'] . dt(" [Y/n]? ");
  $db_use_sitename = densite_readline($readline_str);
  if ($db_use_sitename == 'Y' || empty($db_use_sitename)){
    $result = densite_delete_db($db_spec);
    return $result;
  }
}

// }}}
// {{{ densite_save_dump($db_spec)

/**
 * @param  
 *   array  $db_spec array holding information of the database to dump
 * @param
 *   string $dump_file, the string containing the path to the sql dump to be saved
 * @return 
 *   int on success and 0 on failure
 */ 
function densite_save_dump($db_spec, $dump_file){
  // build command
  // execute it

  if ($db_spec['driver'] == 'mysql' || $db_spec['driver'] == 'mysqli'){
    $command = "mysqldump -u " . $db_spec['username'] . " ";
    $command.= "-p" .        $db_spec['password'] . " ";
    $command.= "-h" .        $db_spec['host'] . " ";
    $command.=               $db_spec['database'] . " > " . $dump_file; 
  } else if ($db_spec['driver'] == 'pgsql'){
    $command = "export PGPASSWORD='$db_spec[password]';";
    $command.= "pg_dump ";
    $command.= "-h " . $db_spec['host'] . " ";
    $command.= "-U " . $db_spec['username'] . " ";
    $command.= $db_spec['database'] . " > " . $dump_file;
  } else {
    return 0;
  }
  densite_exec($command); 
  return 1;
}

// }}}
// {{{ densite_load_sql($db_spec)

/**
 * @param  
 *   array  $db_spec array holding information about the database to load the sql dump into
 * @param
 *   string $dump_file, the string containing the path to the sql dump to be loaded
 * @return 
 *   int on success and 0 on failure
 */ 
function densite_load_dump($db_spec, $dump_file){

  if ($db_spec['driver'] == 'mysql' || $db_spec['driver'] == 'mysqli'){
    $command = "mysql -u " . $db_spec['username'] . " ";
    $command.= "-p" .        $db_spec['password'] . " ";
    $command.= "-h" .        $db_spec['host'] . " ";
    $command.=               $db_spec['database'] . " < " . $dump_file; 
  } else if ($db_spec['driver'] == 'pgsql'){
    $command = "export PGPASSWORD='$db_spec[password]';";
    $command.= "psql ";
    $command.= "-h " . $db_spec['host'] . " ";
    $command.= "-U " . $db_spec['username'] . " ";
    $command.= $db_spec['database'] . " < " . $dump_file;
  } else {
    densite_print(dt("Unknown driver") . "\n");
    return 0;
  }
  densite_exec($command); 
  return 1;
}

// }}}
// {{{ densite_load_db($site, $db_spec, $install)
/**
 * function for populating the newly created database through 
 * readline interface
 *
 * @param 
 *   string $site the drupal website to enable
 * @param 
 *   array $db_spec an array holding the db_spec for the database to be loaded
 * @param
 *   int  $install, indicating if it is an install or not
 * @return
 *   int 1 no loaded db (continue with manual install on web)  
 *       0 is failure and 
 *       2 successfull loaded database
 */
function densite_load_db($site, $db_spec, $install = 0){
  // check to see if user wants to use an sql dump.
  // if 'Yes' get a list of available dumps to use, if 'no' go to web install
  $readline_str = dt("Load a sql dump into database: ") . $db_spec['database'] . dt(" [Y/n]? ");
  $db_use_dump = strtoupper(densite_readline($readline_str));
  
  if ($db_use_dump == 'Y' || empty($db_use_dump)){
    $manual = 0;
    // if we are installing only option will be from a file
    if (!$install){
      $readline_str = dt("Do you want to use an sql dump from a running drupal [s]ite or dump from a [f]ile [s/f]? ");
      $file_or_site = strtoupper(densite_readline($readline_str));
    } else {
      // we presume when making an install there is no active drupal site
      $file_or_site ='F';
    }
    // type of dump to read - From a file located in the commands/densite/sql folder
    switch ($file_or_site) { 
      case 'F' :
      // use a file for generating database
        
      $res = densite_load_db_file($site, $db_spec);
      if (!$res){
        return 0;
      } else {
        if (DENSITE_DEBUG) { print "Returning 2\n"; }
        return 2;
      }
      break;
      // or use a sites sql as a dump for the new site.
      // if we are cloning a site, then all the sql can be
      // imported. 
    case 'S' :
      $res = densite_load_db_site($site, $db_spec);
      if (!$res){
        return 0;
      } else {
        return 2;
      }
      break;
   
    default:
      // wrong option start over
      if (DENSITE_DEBUG) print "else Returning 0\n";
      return 0;
      break;
    }
  }
  // no db loaded, it need to be a manual install
  return 1;
}
  // manual install
// }}}
// {{{ densite_download_dumps

/** 
 * function densite_download_dumps
 * just download some default sql dumps from a default 
 * location if sql folder is empty
 *
 * @return 
 *   int  positive on success and 0 on failure
 */
function densite_download_dumps(){

  $url = "http://www.os-cms.net/sites/default/files";
  $file = "sql"; 
  $filetype = "tar.gz";
  $filelong = $file . "." . $filetype;
  $url_file = $url . "/" . $filelong;
  $sql_dir = dirname(__FILE__) . "/sql";

  #densite_print($url_file. "\n");
  #$dl_dumps = strtoupper(densite_readline("Download and install default dumps (236 KB) for all versions of drupal [Y/n]? "));
  #if ($dl_dumps == 'Y' || empty($dl_dumps)){      
    system("wget " . $url_file);
    system("wget " . $url_file . ".md5" );
    densite_print(dt("Md5 validating of file ") . $filelong . "\n");
    densite_exec("md5sum -c " . $filelong . ".md5");
    densite_exec("tar xvfz " . $filelong);
    densite_exec("cp -Rf ./sql/* " . $sql_dir);  
    densite_exec("rm " . $filelong);
    densite_exec("rm -Rf " . "./" . $file);
    densite_exec("rm " . $filelong . ".md5");
    densite_print(dt("sql dumps downloaded and installed") . "\n");
  #} 

}
// }}}
// {{{ densite_load_db_file($site, $db_spec)

/**
 * function for loadig database throug a file method
 *
 * @param 
 *   string $site the drupal website to enable
 * @param 
 *   array $db_spec an array holding the db_spec specifing the database to load 
 * @return
 *   int 1 on succes 0 on failure
 */
function densite_load_db_file($site, $db_spec){
  // get dump dir    
  $dump_dir = dirname(__FILE__) . "/sql";
  $res = densite_list_sql_dumps(true);
  if (!$res){
    densite_print(dt("Continue with clean install. No dumps were found in.  ") . $dump_dir .  "\n");
    return 1;
    /*
    densite_print(dt("You can choose to download some default dumps found at:" . "\n"));
    $dl_dumps = strtoupper(densite_readline("Download and install default dumps (236 KB) for all versions of drupal [Y/n]? "));
    if ($dl_dumps == 'Y' || empty($dl_dumps)){      
      $dl = densite_download_dumps();
    }*/
  }
  
  $res = densite_list_sql_dumps(true);
  if (!$res){
    return 0;
  }
    
  densite_print(dt("You can populate your database with the following sql dumps:") . "\n");
  densite_list_sql_dumps();
  
  // dumps are found we prompt user to load a dump
  $sql_file = densite_readline(dt("Enter exact name of sql file to use: "));
    
  if (empty($sql_file)){
    // if the user entered a dump that did not exist, we start over again
    densite_print(dt("You need to enter a file name.") . "\n");
    return 0;
  }
  $sql_file = $dump_dir .  "/" . $sql_file;
  if (!file_exists($sql_file)){
    // if the user entered a dump that did not exist, we start over again
    densite_print(dt("File does not exist or is not readable. Try again. ") . "\n");
    return 0;
  }
  // got so far. time to load the dump into the database
  $res = densite_load_dump($db_spec, $sql_file);
  if (!$res){
    densite_print(dt("Could not load dump. ") . $sql_file . dt(" We try again") . "\n");
    return 0;
  } else {
    return 1;
  }
}    

// }}}
// {{{ densite_load_db_site($site, $db_spec)
/**
 * function for populating database with site method
 *
 * @param 
 *   string   $site the drupal website to enable
 * @param 
 *   array    $db_spec an array holding the db_spec specifing the database to load 
 * @return
 *   int      1 on succes 0 on failure
 */
function densite_load_db_site($site, $db_spec){

  densite_print(dt("Listing all sites except: ") . "$site\n");
  densite_list_all_sites($site);
  $readline_str = dt("Enter exact site name to load dump from:") . "\n";
  $load_site = densite_readline($readline_str);
  
  if (empty($load_site)){
    densite_print(dt("You need to enter a site.") . "\n");
    return 0;
  }
  
  if ($load_site == $site){
    densite_print(dt("Can not load anything from the created database for the site we are enabling.") . "\n");
    return 0;
  }
  if (!densite_site_exists($load_site)){
    densite_print(dt("The site to load from does not exist.") . "\n");
    return 0;
  }

  if (densite_sub_exists($load_site)){
    $settings = densite_get_filenames($load_site);
  } else { 
    $settings = densitesa_get_filenames($load_site);
  }

  $driver_spec = 0;
  $save_spec = _drush_sql_get_spec_from_settings($settings['settings_file'], 'source');
  
  // check and see if we load and save from same database type
  if ($save_spec['driver'] != $db_spec['driver']){
    $driver_spec = 0;
  } else {
    $driver_spec = 1;
  }
  if ( $save_spec['driver'] == 'mysql' && $db_spec['driver'] == 'mysqli' ){
    $driver_spec = 1;
  }
  if ( $save_spec['driver'] == 'mysqli' && $db_spec['driver'] == 'mysql' ){
    $driver_spec = 1;
  }

  if ($driver_spec){
    densite_save_dump($save_spec, './dump_file');       
    $res = densite_load_dump($db_spec, './dump_file');
    densite_exec("rm ./dump_file");
    return 1;
  } else {
    densite_print(dt("The database source and target have to be same type.") . "\n");
    return 0;
  }
  return 0;
}

// }}}
// {{{ densite_list_sql_dumps()
/**
 * function for listing all sql dumps
 * @param bool is this is true no printing will be done
 * @param int major_version. If this is set only list the specified version
 * @return int $num_dumps, the count of sql dumps in the folder
 */
function densite_list_sql_dumps($silent = false, $major_version = ''){
  $sql_dir = dirname(__FILE__) . "/sql";
  $sql_dumps = scandir($sql_dir);
  $num_dumps = 0;
  for ($i = 0; $i < count($sql_dumps); ++$i){
    if(!is_file($sql_dir . "/" . $sql_dumps[$i])){
      // skip
    } else {
      $num_dumps++;
      if (!$silent){
        densite_print($sql_dumps[$i] . " ");
      }
    }
  }
  if (!$silent){
    densite_print("\n");
  }
  return $num_dumps;
} 
// }}}
// cron function
// {{{ densite_create_cron_line($site)

/**
 * function for showing a readline form where we can collect 
 * information about how crons should be done
 * 
 * @param string $site
 * @return array $cron with info about the cron
 */
function densite_create_cron_line($site){
  $readline_str = dt("Shall cron run once every [h]our or once every [d]ay or once every month[m]: [h]/[d]/[m]? ");
  $cron = array();
  $h_or_d = densite_readline($readline_str);
  if ($h_or_d != 'h' AND $h_or_d != 'd' AND $h_or_d != 'm'){
    return 0;
  }
  
  $readline_str = dt("Enter minute when your cron shall run. Must be between 0 and 59: ");
  $cron = array();
  $minute = (int)densite_readline($readline_str);
  if ($minute >= 0 && $minute <= 59){
    $cron['minute'] = $minute;
  } else {
    return 0;
  }

  if ($h_or_d == 'h') return $cron;
  
  $readline_str = dt("Enter hour when your cron shall run. Must be between 0 and 23: ");
  $hour = (int)densite_readline($readline_str);
  if ($hour >= 0 && $hour <= 23){
    $cron['hour'] = $hour;
  } else {
    return 0;
  }
  
  if ($h_or_d == 'd') return $cron;
  
  $readline_str = dt("Enter day in month when your cron shall run. Must be between 1 and 31: ");
  $day = (int)densite_readline($readline_str);
  if ($day >= 1 && $day <= 31){
    $cron['day'] = $day;
  } else {
    return 0;
  }
  
  if ($h_or_d == 'm') return $cron;
  
  return 0;
}

// }}} 
// {{{ densite_disable_cron($site)

/**
 * function for deleting entries from the crontab file
 *
 * @param string $site
 * @return int 1 
 */
function densite_disable_cron($site){

  $cron_line = "http://" . $site . "/cron.php";
  $output = $cron_str = '';

  // get contents of crontab. We use exec because we need the output
  // do the crontab -l command to get old crontabs
  exec("crontab -l", $output, $res);
  foreach ($output as $line) {
    if (!preg_match("[$cron_line]", $line)){
      $cron_str.=$line . "\n";
    }
  }

  $cron_str = rtrim ($cron_str);
  $command = 'echo "' . $cron_str . '" | crontab -';
  densite_exec($command);
  return 1;
}

// }}}
// {{{ densite_enable_cron($site)
/**
 * function for enabling the site in the crontab
 *
 * @param string $site
 * @return int 0 on failure 1 on success
 */
function densite_enable_cron($site){
  $readline_str = dt("Do you want to enable cron job for the site [Y/n]? ");
  $y_or_n = strtoupper(densite_readline($readline_str));
  if ($y_or_n == 'Y' || empty($y_or_n)){
    // continue
  } else {
    return 0;
  }
  
  $cron = densite_create_cron_line($site);
  if (!$cron){
    densite_print("Could not enable cron");
    return 0;
  } else {
    // minute always set in this scenario
    $cron_line = $cron['minute'] . " ";
    // but not the day
    if (isset($cron['hour'])){
      $cron_line.= $cron['hour'] . " ";
    } else {
      $cron_line.= "* ";
    }
    // and not the month
    if (isset($cron['day'])){
      $cron_line.= $cron['day'] . " ";
    } else {
      $cron_line.= "* ";
    }
    // add the remains
    $cron_line.="* *  ";
  }
  
  $cron_line.=  DENSITE_CRON_COMMAND . " " . "http://" . $site . "/cron.php";

  // get contents of crontab. We use exec because we need the output
  // do the crontab -l command to get old crontabs
  exec("crontab -l", $output, $res);
  $output = densite_print($output, 1);
  $output.= $cron_line;
  $command = 'echo "' . $output . '" | crontab -';
  densite_exec($command);

  return 1;
}

// }}}
// misc helper functions
// {{{ densite_get_settings_filename ($site)

/**
 * function for getting settings filenames when enabling a sub-site
 * @param   string  $site
 * @return  string  $settings_filename
 */ 
function densite_get_settings_filename($site){
  $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
  $settings_filename = $drupal_root . "/sites/" . $site . "/settings.php";
  return $settings_filename;
}

// }}}
// {{{ densite_get_sites_root($path)

/**
 *  function for finding base_path of all sites
 *  @param
 *    string $path the path to search in
 *  @return 
 *    string $base the base_path of all sites
 */ 
function densite_get_sites_root($path){
  $str = DENSITE_WWW_HOME;
  if (!empty($str)){
    return $str;
  } 
  // we remove the /htdocs level: we have e.g. 
  // /home/user/www/www.www.os-cms.net
  // we remove the base site level: we have e.g. 
  // /home/user/www 
  $path = explode('/', $path);
  array_pop($path); 
  array_pop($path); 
  $base = implode ('/', $path);
  return $base;
}

// }}}
// {{{ densite_get_filenames($site)

/**
 * function for creating an array of the most used names when configuring 
 * the drupal site and the apache server
 * 
 * @param
 *   string   $site
 * @return
 *   array    $filenames  an array of filenames
 */
function densite_get_filenames($site){
  $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');

  $settings_file = densite_get_settings_filename($site);
  $www_base = densite_get_sites_root($drupal_root);
  
  // find the default apache2.conf file
  $path = explode('/', __FILE__); array_pop($path); 
  $apache_default_conf = implode ('/', $path) . "/apache2/" . DENSITE_APACHE_FILE;
  
  $major_version = substr(drush_drupal_version(), 0, 1); 
  $settings_file_default = implode ('/', $path) . "/settings/default.settings.php-" . $major_version . ".x" ;

  $filenames = array(
    'drupal_root'            => $drupal_root,
    'drush_command'          => $drush_command,
    'settings_file'          => $settings_file, 
    'settings_dir'           => $drupal_root . "/sites/" . $site,
    'settings_file_default'  => $settings_file_default,
    'files_dir'              => $drupal_root . '/sites/' . $site . '/files',
    'www_base'               => $www_base,
    'www_base_site'          => $www_base . "/" . $site,
    'www_base_htdocs'        => $www_base . "/" . $site . "/". DENSITE_APACHE_HTDOCS,
    'www_base_site_logs'     => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS,
    'www_base_site_access'   => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS . "/access.log",
    'www_base_site_error'    => $www_base . "/" . $site . "/" . DENSITE_APACHE_LOGS . "/error.log",
    'apache_default_conf'    => $apache_default_conf,
    'apache_sites_avail'     => DENSITE_APACHE_SITES_AVAIL . "/" . $site,
  );
  return $filenames;
}

// }}}
// {{{ densite_write_settings($setttings_file, $settings)

/**
 * function for writing to a file and controlling how to exit on error
 * 
 * @param
 *   string $settings_file
 * @param
 *   string $settings to write
 */
function densite_write_settings($settings_file, $settings){
  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');

  if ($simulate){
    densite_print(SIMULATE_PREPEND . dt("Writing new settings.php file: " . $settings_file) . "\n");
    return 1;
  }

  $res = densite_write_file($settings_file, $settings);
  if (!$res){
    if (empty($silent)){ 
      densite_print(ERROR_PREPEND. dt("Could not write new settings.php: " . $settings_file) . "\n");
    }  
    if (empty($force)){
      exit(1);
    }
  } else {
    if (empty($silent)){
      densite_print(NO_ERROR_PREPEND . dt("Writing new settings.php file: " . $settings_file) . "\n");
    }
  }
}

// }}}
// {{{ densite_list_all_sites()

/**
 * @param   string $exclude, an optional site to exclude form the search
 *          We only operate at a site at a time so nice to be 
 *          able to exclude a site from listings etc.
 *
 *   function for listing all the sites in the sites/ directory
 */
function densite_list_all_sites($exclude = ''){
  $res = densite_init();
  if (!$res){
    return $res;
  }
  
  $all_sites = array();
  $all_sites = densite_get_all_sites($exclude);
  foreach ($all_sites as $site){
    densite_print($site['base'] . " serves: "); 
    foreach ($site['sub'] as $sub_site){
      densite_print($sub_site . " ");
    }
    densite_print("\n");
  }
}

// }}}
// {{{ densite_list_sub_sites()

/**
 * function for listing all the sites in the sites/ directory
 */
function densite_list_sub_sites(){
  $sub_sites = array();
  $sub_sites = densite_get_sub_sites();
  foreach ($sub_sites as $site){
    densite_print($site . " "); // " has the following sub: \n";
  }
  densite_print("\n");
}

// }}}
// {{{ densite_get_all_sites()

/**
 * @param   
 *   string $exclude, an optional site to exclude form the search
 *   We only operate at a site at a time so nice to be 
 *   able to exclude a site from listings etc.
 * @return  
 *   array $sites an array with base sites containing an array of all 
 *   the base sites sub sites
 */
function densite_get_all_sites($exclude =''){
  // get all base sites
  $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
  $drupal_sites_path = densite_get_sites_root($drupal_root);
  $sites = array();

  $sites_dir = scandir($drupal_sites_path);
  for ($i = 0; $i < count($sites_dir); ++$i){
    $settings_file = $drupal_sites_path . "/" . $sites_dir[$i] . "/" . DENSITE_APACHE_HTDOCS .  "/sites/default/settings.php";
    
    if($sites_dir[$i] == '.'){
      // skip current dir
    } else if (!is_dir($drupal_sites_path . "/" . $sites_dir[$i])){
      // skip if not a dir
    } else if (!file_exists($settings_file)){
      // skip if no default settings file was found
    } else if ($sites_dir[$i] == '..') {
      // skip if it is base dir
    } else if ($sites_dir[$i] == 'default') {
      // skip if it is default dir
    } else if ($sites_dir[$i] == 'all') {
      // skip if dir is all
    } else if ($sites_dir[$i] == $exclude) {
      // skip if dir should be excluded
    } else {
      // grab the base if it is a base
      $sites[$i]['base'] = $sites_dir[$i];
      $sites[$i]['sub'] = densite_get_sub_sites($sites_dir[$i], $exclude);
    }
  }
  return $sites;
}

// }}}
// {{{ densite_get_base_site($sub);

/**
 * function for getting a base site to a sub-site
 * 
 * @param string $sub name of the sub-site
 * @return mixed 
 *  string $base name of the base site or
 *  0 if no base site could be found
 */
function densite_get_base_site($sub){
  
  $all_sites = array();
  $all_sites = densite_get_all_sites();
  foreach ($all_sites as $site){
    foreach ($site['sub'] as $sub_site){
      $base = $site['base'];
      if ($sub_site == $sub){
        return $base;
      }
    }
  }
  return 0;
}

// }}}
// {{{ densite_is_current_base($site)

/**
 * function to determinate if the site where we execute the densite script
 * is the actual base site
 * @param string $base the site to examine
 */

// FIX: Do we use this function
function densite_is_current_base($base){
  $settings_file = $_SERVER['PWD'] . "/sites/default/settings.php";
  if (file_exists($settings_file)){
    return 1;
  } else {
    return 0;
  }
}
// }}}
// {{{ densite_sub_to_current($site)

/**
 * function to determine if a site is sub to current site
 * @param string $sub the sub-site
 * @return int 1 if it is and 0 if it is not
 */
function densite_sub_to_current($sub){
  $settings_dir = drush_get_context('DRUSH_DRUPAL_ROOT') . "/sites/" . $sub;
  if (file_exists($settings_dir)){
    return 1;
  } else {
    return 0;
  }

}
// }}}
// {{{ densite_site_exists ($site, $exclude)

/**
 * function for determing if a site exists
 * 
 * @param string $site the site to examine
 * @param string $exclude a site to exclude
 * @return int 1 if it exists and 0 if it does not
 */
function densite_site_exists($site, $exclude){
  if (densite_base_exists($site, $exclude)){
    return 1;
  }
  if (densite_sub_exists($site, $exclude)){
    return 1;
  }
  return 0; 
}

// }}}
// {{{ densite_base_exists($site)


/**
 * function for determing if a base site exists
 * 
 * @param string $site the site to examine
 * @param string $exclude a site to exclude
 * @return int 1 if it exists and 0 if it does not
 */
function densite_base_exists($base, $exclude = ''){
  $all_sites = array();
  $all_sites = densite_get_all_sites();
  $exists = 0;
  foreach ($all_sites as $site){
    foreach ($site as $base_site){
      if ($base_site == $base){
        $exists = 1;
      }
      if (!empty($exclude) && $base_site == $exclude){
        $exists = 0;
      }
    }
  }
  return $exists;
}

// }}}
// {{{ densite_sub_exists($site)

/**
 * function to determine if a sub-site exists
 * 
 * @param string $site the site to examine
 * @param string $exclude a site to exclude
 * @return int 1 if it exists and 0 if it does not
 */
function densite_sub_exists($sub, $exclude = ''){
  $all_sites = array();
  $all_sites = densite_get_all_sites();
  $exists = 0;
  foreach ($all_sites as $site){
    foreach ($site['sub'] as $sub_site){
      if ($sub_site == $sub){
        $exists = 1;
      }
      if (isset($exclude) && $sub_site == $exclude){
        $exists = 0;
      }
    }
  }
  return $exists;
}
// }}}
// {{{ densite_get_sub_sites($base)

/**
 * @param
 *   string $exclude, an optional site to exclude form the search
 * @return 
 *   array $sites an array of all the base sites, which each can serve
 *         as multisite environments 
 */
function densite_get_sub_sites($base = '', $exclude = ''){
  // get local sites for the base install where you are standing 
  $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
  if (empty($base)){
    $drupal_sites_path = $drupal_root . "/sites";
    $sites = array();
  } else {
    $sites_root = densite_get_sites_root($drupal_root);
    $drupal_sites_path = $sites_root . "/" . $base . "/" . DENSITE_APACHE_HTDOCS . "/sites";
  }

  $sites_dir = scandir($drupal_sites_path);
  for ($i = 0; $i < count($sites_dir); ++$i){
    $settings_file = $drupal_sites_path . "/" . $sites_dir[$i] . "/settings.php";
    
    if($sites_dir[$i] == '.'){
      // skip current dir
    } else if (!is_dir($drupal_sites_path . "/" . $sites_dir[$i])){
      // skip if not a dir
    } else if (!file_exists($settings_file)){
      // skip if no settings file was found
    } else if ($sites_dir[$i] == '..') {
      // skip if it is base dir
    } else if ($sites_dir[$i] == 'default') {
      // skip if it is default dir
    } else if ($sites_dir[$i] == 'all') {
      // skip if dir is all
    } else if ($sites_dir[$i] == $exclude) {
      // skip if dir is excluded
    } else {
      // grab the dir
      $sites[$i] = $sites_dir[$i];
    }
  }
  return $sites;

}

// }}}
// {{{ densite_get_file($filename)

/**
 * @param 
 *  string $filename The filename to open and read
 * @return 
 *  string $content of the file which has been read 
*/
function densite_get_file($filename){
  $force = drush_get_option('f');
  $silent = drush_get_option('s');
  $simulate = drush_get_option('simulate');

  if ($simulate){
    densite_print(dt("Will try and read contents of $filename") . "\n");
    return 1;
  }
  
  clearstatcache();
  $content = fread($fp = fopen($filename, 'r'), max(1, filesize($filename))); 
  fclose($fp);
  return $content;
}

// }}}
// {{{ densite_write_file($filename, $contents)

/**
 * @param
 *  string $filename The filename to write
 * @param
 *  string $contents the content to write
 */
function densite_write_file($filename, $contents) {
  if ($fp = @fopen($filename, 'w')) {
    flock($fp,2);
    fwrite($fp, $contents);
    flock($fp,3);
    fclose($fp);
    return 1;
  } else {
    return 0;
  }
}

// }}}
// {{{ densite_readline($line)

/**
 * funtion for wrapping the readline function
 * @param string ouput to print to screen
 * @return string the input which readline reads
 */
function densite_readline($line){
  if (function_exists(readline) ){  
    $ret = readline($line);
    return $ret;
  } else { 
    // if not readline we use this
    // we could actually just use this .)
    // found at php.net
    print $line;
    $out = "";
    $key = "";
    $key = fgetc(STDIN);        //read from standard input (keyboard)
    while ($key!="\n") {       //if the newline character has not yet arrived read another
      $out.= $key;
      $key = fread(STDIN, 1);
    }
    return $out;
  }
}

// }}}
// {{{ densite_test()

/**
 * function for testing
 */
function densite_test($site){
  /*
  $db_spec = array();
  $db_spec = drush_get_context('DRUSH_DB_CREDENTIALS');
  print_r($db_spec);
  $db_spec = densite_get_db_spec();  
  print_r($db_spec);
   */
  densite_download_dumps();
}

  #$drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
  /*
  //$settings = densite_get_filenames($site);
  //print_r($settings);
  require_once $drupal_root . "/includes/common.inc";
  require_once $drupal_root . "/modules/system/system.install";
  //print $drupal_root;
  require_once $drupal_root . "/includes/install.inc";
  drupal_uninstall_schema();
   */
// }}}

function densite_update_etc_hosts ($hostname){
    // create new hosts file and reload server
    $hosts_file_str = file_get_contents("/etc/hosts");
    $new_host = "127.0.0.1\t$hostname\n";
    if (!strstr($hosts_file_str, $new_host)) {
      $new_hosts_file_str =  $new_host . $hosts_file_str;
      file_put_contents("/tmp/hosts", $new_hosts_file_str);
      densite_exec("sudo cp -f /tmp/hosts /etc/hosts");
      densite_exec("sudo /etc/init.d/apache2 reload");
      densite_exec("rm /tmp/hosts");
    }
}