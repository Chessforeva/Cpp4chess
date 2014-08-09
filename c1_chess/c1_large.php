<?
//
// Splits large PGN files.
// Generates HTML index and browsable chess pages
//
// Extender for c1_chess
// Compile to exe with BamCompile
//
//
//

$GAMES_PER_BROWSABLE = 100;

$web = "Web".time();
mkdir ($web, 0700);

$cur = getcwd();
$dir = dir( $cur );

$CR = chr(13).chr(10);

echo "New folder ".$web.$CR;

$path = $cur."/".$web."/";
$I = fopen ($path."index.htm", "w");
fputs($I,"<html><body>".$CR);

$fc = 0;
while (false !== ($entry = $dir->read()))
{
    if( substr($entry,0,1)!="." && !(is_dir($cur."/".$entry)) )
    {

    $ax = strrpos($entry,".");
    $ext = ( $ax>0 && $ax == strlen($entry)-4 ? substr($entry,$ax+1) : "" );

    if( strlen($ext)==0 || strpos("{exe}{dll}{com}",$ext)==false)
    {
    $fc++;          
    $f = fopen ($cur."/".$entry, "r");
    $h = false;
    $hev = false;
    $N = 0;
    $gc = 0;
    $nw = true;
    $o = "";
    $gm = false;
    $rLast = 0;
    $r1 = 0;
    $r2 = 0;
    while (!feof ($f))
        {
        $s = trim( fgets($f, 4096) );
        $l = strlen( $s );
        if($l==0) $o.=$CR;
        else
            {
            $t = ( substr($s,0,1)=="[" && substr($s,$l-1,1)=="]" );
            if($t)      // header block
                {
                if(!$h)     // aha, this is a pgn file
                    {
                    $h=true;
                    echo $entry."\n";
                    }
                }
            if(!$h) break;           // this is not a pgn file
            
            if($gm)     // write this game
                {
                $gm = false;
                fputs( $z, $o );
                $o = "";
                } 
            $o.=$s.$CR;
                            
            if($t)
                {
                if( strpos($s,'"') == strrpos($d,'"') ) $d = "";
                else
                    {
                    $d = substr($s, strpos($s,'"')+1 );
                    $d = substr($d, 0, strrpos($d,'"') );   // value of header
                    }
                if( substr($s,0,7)=="[Event " && $hev==false )
                    {
                    fputs($I,$d."<br>".$CR);
                    $hev = true;
                    }
                if( substr($s,0,7)=="[White " ) $gc++;
        
                if( substr($s,0,7)=="[Round " )
                    {
                    $r = intval( $d );
                                            
                    if($gc * ( 10 / 8 ) > $GAMES_PER_BROWSABLE )
                        {
                        if($r!=$rLast) $nw = true;
                        }
                    if($gc * ( 6 / 10 ) > $GAMES_PER_BROWSABLE )
                        {
                        $nw = true;
                        }
                    if($r1==0)
                        {
                        $r1 = $r;
                        $r2 = $r;
                        }
                    else if(!$nw) $r2 = $r;
                                                     
                    $rLast = $r;   
                    }
                }
            else
                {
                // not a header block
                $gm = true;
                }
                
            if($nw)     // new pgn file part
                {
                if($N>0)
                    {
                    $fname = $fn;
                    $m = strrpos($fname,".");
                    if( $m>0 && $m >= strlen($fname)-4 ) $fname = substr($fname,0,$m);
                    
                    fclose($z);
                    fputs($I,'<a href="'.$fname.'.htm">Round '.$r1.
                            ($r2>$r1 ? "-".$r2 : "" ).'</a> ');
                    $r1 = 0;
                    $r2 = 0;
                    exec("c1_chess.exe ".$path.$fn." /C=400");
                    unlink($path.$fn);
                    }
                              
                $p1 = "_part_".($N+1);
                echo " part ".($N+1).$CR;
                
                if( strlen($ext)==0 ) $fn = trim($entry).$p1;
                else $fn = trim(substr($entry,0,$ax)).$p1.".".$ext;                       
                            
                $z = fopen ($path.$fn, "w");
                $N++;
                $nw = false;
                $gc = 0;
                }                
            }  
        }

    if($N>0)
        {
        
        fputs( $z, $o );
        fclose($z);
        
        if($N==1)
            {
            $fn1 = str_replace($p1,"",$fn);
            rename($path.$fn, $path.$fn1);
            $fn = $fn1;
            }
        $fname = $fn;
        
        $m = strrpos($fname,".");
        if( $m>0 && $m >= strlen($fname)-4 ) $fname = substr($fname,0,$m);        
        
        fputs($I,'<a href="'.$fname.'.htm">Round '.$r1.
                ($r2>$r1 ? "-".$r2 : "" ).'</a> '."<br>");
        exec("c1_chess.exe ".$path.$fn." /C=400");
        unlink($path.$fn);
        }
     
    fclose($f);
    }
    }
}
$dir->close();

if($fc==0)
    {
    fputs($I,"Put PGN files in current folder and this tool will generate browsables.".$CR);
    }
fputs($I,"</body></html>".$CR);
fclose($I);

exec("start ".$path."index.htm");

?>
