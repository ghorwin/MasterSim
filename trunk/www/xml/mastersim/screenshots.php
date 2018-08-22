<?php
session_start();

if (isset($_GET['pi-on']))
	phpinfo();
	
//Message variable initialisieren
$strMsg="";

//configfile laden
$strPfad="2nd/php/init.inc.php";
while (!file_exists($strPfad))
	$strPfad='../'.$strPfad;
require($strPfad);

if (isset($_GET['err-on'])) {
	error_reporting(E_ALL);
}


include(VERZ_DOMAIN_PHP."2nd/php/funk.inc.php");
//SPRACHE FESTLEGEN
if (isset($_REQUEST['aLa']))
	$_SESSION['aLa']=$_REQUEST['aLa'];

if (!isset($_SESSION['aLa']))
	$_SESSION['aLa']="de";

//angebot auslesen
//automatisch erkennen
if(preg_match('/^'.bedif(VERZ_DOMAIN_HTTP,'\/'.trim(VERZ_DOMAIN_HTTP,"/")).'\/(.*)\//U',$_SERVER['PHP_SELF'],$arrTemp)>0)
	$_ANGEBOT=$arrTemp[1];
else
	$_ANGEBOT="";

unset($arrTemp);

//Post und get daten überprüfen! wegen SQL injection und anderen bösewichtern
$arrNot=explode('|','<|>|[|]|;|\'|"|=');
foreach($_GET as $argk=>$argv)
	$_GET[$argk]=str_replace($arrNot,'',$argv);
foreach($_POST as $argk=>$argv)
	$_POST[$argk]=str_replace($arrNot,'',$argv);
	
//try new post ding


set_include_path(
get_include_path()
. PATH_SEPARATOR
. '/homepages/10/d194802226/htdocs/Bauklimatik/2nd/php/phpids-0.6/lib'
);

require_once 'IDS/Init.php';

$request = array(
	'REQUEST' => $_REQUEST,
	'GET' => $_GET,
	'POST' => $_POST,
	'COOKIE' => $_COOKIE
);
$init = IDS_Init::init('/homepages/10/d194802226/htdocs/Bauklimatik/2nd/php/phpids-0.6/lib/IDS/Config/Config.ini');
$ids = new IDS_Monitor($request, $init);
require_once 'IDS/Log/File.php';
require_once 'IDS/Log/Composite.php';

$compositeLog = new IDS_Log_Composite();
$compositeLog->addLogger( IDS_Log_File::getInstance($init) );

require_once 'IDS/Log/Email.php';
$compositeLog->addLogger( IDS_Log_Email::getInstance($init) );

$result = $ids->run();

if (!$result->isEmpty()) {
	// Take a look at the result object
	//echo "RESULT:".$result;
	//$compositeLog->execute($result);
}

if ($result->getImpact()>10) {
	// Take a look at the result object
	//echo "RESULT:".$result;
	$compositeLog->execute($result);
}



//datei laden
if (file_exists(substr(VERZ_SERVER_PHP,0,-1).str_replace(".php",".xml",$_SERVER['PHP_SELF']))) {
	$objSeite=xml_datei_laden(substr(VERZ_SERVER_PHP,0,-1).str_replace(".php",".xml",$_SERVER['PHP_SELF']));
	if (!is_object($objSeite)) {
		$strMsg.="<br><span class=\"errorMsg\">Fehler #3: We are sorry but we could not find the content for the requested page!</span>";
	}
}
else
		$strMsg.="<br><span class=\"errorMsg\">Fehler #4: We are sorry but this file does not exist!</span>";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" lang="<?php ECHO $_SESSION['aLa']; ?>">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/><meta name="verify-v1" content="vJjp9Sj+ygOG3Ad9Tw90AMDOXrbfh9GsWSqnWc36jyQ=" />
<meta http-equiv="content-language" content="<?php echo $_SESSION['aLa']; ?>">
<?php
	// alle Seiten bekommen zuerst allgemein.css
	echo '<link rel=stylesheet type="text/css" href="/'.VERZ_DOMAIN_HTTP.'2nd/css/allgemein.css">';
if($_ANGEBOT=="cond" || $_ANGEBOT=='delphin' || $_ANGEBOT=='nandrad' || $_ANGEBOT=='therakles' || $_ANGEBOT=='mastersim' || $_ANGEBOT=='postproc') {
	// die Software-Angebote bekommen als Nächstes die software.css und Ihre lokalen 'overrides'
	echo '<link rel=stylesheet type="text/css" href="/'.VERZ_DOMAIN_HTTP.'2nd/css/software.css">';
	echo '<link rel=stylesheet type="text/css" href="/'.VERZ_DOMAIN_HTTP.$_ANGEBOT.'/2nd/css/'.$_ANGEBOT.'.css">';
}
else  {
	// alle anderen (top-level) Seiten bekommen das Top-Level override
	echo '<link rel=stylesheet type="text/css" href="/'.VERZ_DOMAIN_HTTP.'2nd/css/toplevel.css">';
}
	
echo '<title>'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</title>';
?>

<link rel="stylesheet" type="text/css" href="//cdnjs.cloudflare.com/ajax/libs/cookieconsent2/3.0.3/cookieconsent.min.css" />
<script src="//cdnjs.cloudflare.com/ajax/libs/cookieconsent2/3.0.3/cookieconsent.min.js"></script>
<script>
window.addEventListener("load", function(){
window.cookieconsent.initialise({
  "palette": {
    "popup": {
      "background": "#237afc"
    },
    "button": {
      "background": "#fff",
      "text": "#237afc"
    }
  },
  "position": "top",
  "content": {
    "dismiss": "Accept",
    "link": "http://bauklimatik-dresden.de/",
    "href": "http://bauklimatik-dresden.de/datenschutz.php"
  }
})});
</script>
</head>
<body leftmargin="0" topmargin="0">
<table width=100% border="0" cellspacing="0" cellpadding="0">
	<tr>
		<td>
		<center>
		<table width="<?php echo CSS_OUTER_WIDTH; ?>" border="0" cellspacing="0" cellpadding="0" style="text-align:left">
			<?php
			if ($_SERVER['PHP_SELF']!=VERZ_DOMAIN_HTTP."index.php") {
				echo '<tr><td class="ibkreiter-td">';
				inhalt_laden(VERZ_DOMAIN_PHP."2nd/shtml/kopf-ibk");
				echo '</td></tr>';
			}
			?>
			<tr>
				<td valign="top" class="programm-rahmen"><table width=100% border="0" cellpadding="0" cellspacing="0" class="tableProgrammreiter">
					<tr><td width="<?php echo (CSS_OUTER_WIDTH-200);?>" valign="bottom" class="tdProgrammreiter"><?php
					//header für spez sprache gesetzt..?
					//delphin macht den header ganz selbst...
					if(($_ANGEBOT=='delphin')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'delphin/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'delphin/2nd/images/icon.png" style="padding-right:4px;">DELPHIN</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					elseif(($_ANGEBOT=='therakles')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'therakles/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'therakles/2nd/images/icon.png" style="padding-right:4px;">THERAKLES</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					elseif(($_ANGEBOT=='nandrad')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'nandrad/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'nandrad/2nd/images/icon.png" style="padding-right:4px;">NANDRAD</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					elseif(($_ANGEBOT=='mastersim')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'mastersim/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'mastersim/2nd/images/icon.png" style="padding-right:4px;">MASTERSIM</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					elseif(($_ANGEBOT=='postproc')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'postproc/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'postproc/2nd/images/icon.png" style="padding-right:4px;">POSTPROC</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					elseif(($_ANGEBOT=='cond')&&($_SERVER['PHP_SELF']!='/'.VERZ_DOMAIN_HTTP.'cond/index.php'))
						{
						echo '<table height=100% cellpadding="0" cellspacing="0"><tr><td valign="top"><span class="h1-delphinlogo"><img src="/'.VERZ_DOMAIN_HTTP.'cond/2nd/images/icon.png" style="padding-right:4px;">COND</span></td></tr><tr><td valign="bottom"><span class="h1">'.$objSeite->seite->{'titel'.$_SESSION['aLa']}.'</span></td></tr></table>';
						}
					else
						{
						if (isset($objSeite->seite->{'header'.$_SESSION['aLa']}))
							echo $objSeite->seite->{'header'.$_SESSION['aLa']};
						
						}
					?></td>
					<?php
					//menu laden... wenn kein menu dann wird td ausegspart und mehr platz
					inhalt_laden(VERZ_DOMAIN_PHP."2nd/php/menu.inc.php"); ?>
					</tr>
					<tr><td class="tdProgramminhalt" colspan="2"><?php
					if($_SERVER['PHP_SELF']!="/index.php")
						{
						echo "&nbsp;&nbsp;&nbsp;&nbsp;<span class=\"small\"><a class=\"menu-pos\" href=\"/".VERZ_DOMAIN_HTTP."index.php\">START</a>";
						
						$arrOrdner=explode('/',substr(str_replace(VERZ_DOMAIN_HTTP,'',$_SERVER['PHP_SELF']),1));
						$strPfad="";
						foreach($arrOrdner as $k=>$v)
							{
							//echo $strPfad.$v;
							if(substr($v,-4)!=".php")
								{
								//datei suchen
								if(($arrOrdner[$k+1]!="index.php")&&(file_exists(VERZ_DOMAIN_PHP.$strPfad.$v."/index.xml"))&&(file_exists(VERZ_DOMAIN_PHP.$strPfad.$v."/index.php")))
									{
									$objSeite2=xml_datei_laden(VERZ_DOMAIN_PHP.$strPfad.$v."/index.xml");
									$objSeite2->httppfad=$strPfad.$v."/index.php";
									}
								elseif((file_exists(VERZ_DOMAIN_PHP.$strPfad.$v.".xml"))&&(file_exists(VERZ_DOMAIN_PHP.$strPfad.$v.".php")))
								//versuchen dass datei wie ordner heisst,,,,
									{
									$objSeite2=xml_datei_laden(VERZ_DOMAIN_PHP.$strPfad.$v.".xml");
									$objSeite2->httppfad=$strPfad.$v.".php";
									}
								$strPfad.=$v."/";
								if(isset($objSeite2) && is_object($objSeite2))
									echo "&#187;<a class=\"menu-pos\" href=\"/".VERZ_DOMAIN_HTTP.$objSeite2->httppfad."\">".strtoupper($objSeite2->seite->{'titel'.$_SESSION['aLa']})."</a>";
								unset($objSeite2);
								}
							else
								echo "&#187;<span class=\"menu-pos\">".strtoupper($objSeite->seite->{'titel'.$_SESSION['aLa']})."</span>";
							}
						}
					echo "</span><table width=100% cellpadding=\"5\" border=\"0\">";
					$strFluss="";$strFlussEnde="";$strFlussLinks="";
					foreach($objSeite->content->artikel as $k=>$objDaten)
						{
						if(strstr($objDaten->lang,$_SESSION['aLa']))
							{
							$objDaten->inhalt=trim($objDaten->inhalt, " ");
							$objDaten->titel=trim($objDaten->titel, " ");
							$objDaten->inhalt=text_formatieren($objDaten->inhalt);
							
							if($objDaten->position=="links")
								{
								$strFlussLinks.=bedIf($objDaten->titel,"<h2>".$objDaten->titel."</h2>");
								if(!substr($objDaten->inhalt,0,1)=='<')
									$strFlussLinks.="<p>";
								$strFlussLinks.="<p>".$objDaten->inhalt."</p><span class=\"separator\">&nbsp;</a>";
								}
							elseif($objDaten->position=="rechts")
								{
								$strFlussEnde.=bedIf($objDaten->titel,"<h2>".$objDaten->titel."</h2>");
								if(!substr($objDaten->inhalt,0,1)=='<')
									$strFlussEnde.="<p>";
								$strFlussEnde.="<p>".$objDaten->inhalt."</p><span class=\"separator\">&nbsp;</span>";
								}
							elseif($objDaten->position=="beide")
								{
								if($strFlussLinks!="" || $strFlussEnde!="")
									{
									echo "<tr><td valign=\"top\" width=50%>$strFlussLinks</td><td width=\"40\">&nbsp;</td><td width=50% valign=\"top\">$strFlussEnde</td></tr>";
									}
								
								echo "<tr>";
								echo "<td colspan=\"3\">";
								echo bedIf($objDaten->titel, "<span class=\"h2\">".$objDaten->titel."</span>");
								
								$strFluss="";//moment text anhand des tags  <sbr> splitten
								if(stristr($objDaten->inhalt,"<sbr>"))
									{
									$arrTemp=explode("<sbr>",$objDaten->inhalt);
						
									$objDaten->inhalt="</td></tr><tr>";
									foreach($arrTemp as $k9=>$v)
										{
										$objDaten->inhalt=$objDaten->inhalt."<td valign=\"top\" width=48%><p>".$v."</p></td>";
										
										if((count($arrTemp)-1)>$k9)
											$objDaten->inhalt=$objDaten->inhalt."<td></td>";
										}
 									$objDaten->inhalt=$objDaten->inhalt."</tr>";
 									echo $objDaten->inhalt;
									}
								else
 									{
									if(substr($objDaten->inhalt,0,1)!='<')
										$strFluss.="<p>";
										
									$strFluss.=$objDaten->inhalt."</p>";
									$strFluss.="</td></tr>";
									echo $strFluss;
									}
								$strFluss="";$strFlussLinks="";$strFlussEnde="";
								}
							else		
								{
								$strFlussLinks.=bedif($objDaten->titel,"<h2>".$objDaten->titel."</h2>");
								if(!substr($objDaten->inhalt,0,1)=='<')
									$strFlussLinks.="<p>";
								$strFlussLinks.="<p>".$objDaten->inhalt."</p>";
								}					
							}
						}
					echo "<tr><td width=48% valign=\"top\">$strFlussLinks</td><td width=\"40\">&nbsp;</td><td width=48% valign=\"top\">$strFlussEnde</td></tr>";
					echo "</table>";
				
					?></td>
			</tr>
			
			<!--if(strlen($strMsg)>3)
				echo "<tr><td colspan=\"3\"><p>".$strMsg."</p></td></tr>";
			?>-->


		</table></center></td>
	</tr>
</table>
<!--<div id="footer_nav">
    <?php inhalt_laden(VERZ_DOMAIN_PHP."2nd/shtml/fuss-ibk"); ?>
</div>
-->
<div id="divUber" name="divUber" onclick="{bigPig.src='';document.getElementById('divUber').style.visibility = 'hidden';}">
<table cellpadding="0" cellspacing="0" border="0" width=100% height=100%>
<tr><td valign="middle" align="center">
<table class="tableUber"><tr><td></td><td valign="middle"><center><img name="bigPig" id="bigPig" class="no-opacity"><span class="small"><br>(Auf Hintergrund klicken, um zu schließen)</span></center></td><td></td></tr>
</table>
</td></tr>
</table>
</div>
</body>
</html>
