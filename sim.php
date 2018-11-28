<?php 
if($_SERVER ['REQUEST_METHOD'] == 'GET') {// kiểm tra nếu người dùng đã ấn nút Đăng ký 	
	include('inc/myconnect.php');	
	//error_".$sr."_report_s1ing(0);	
	$s1= $_GET['s1'];
	$s2= $_GET['s2'];
	$s3= $_GET['s3'];
	$s4= $_GET['s4'];
	$m= $_GET['m'];
	$sr= $_GET['sr'];
	$si = $_GET['sig'];		//ok
	$dr = $_GET['dr'];		//ok
	$gprs = $_GET['gprs'];	//ok
	$cnn = $_GET['cnn'];	//ok
	$cne = $_GET['cne'];	//ok
	$b   = $_GET['b'];		//ok
	$h = $_GET['h'];
	$o1 = $_GET['o1'];
	$i2 = $_GET['i2'];

	$st = mysqli_fetch_array( mysqli_query($dbc, "SELECT * FROM ".$sr."_status") );

	if($si != null) 	mysqli_query($dbc, "UPDATE ".$sr."_status SET sig='$si' WHERE id=1");
	if($dr != null) 	mysqli_query($dbc, "INSERT INTO ".$sr."_dr(dr) VALUES ('$dr')");
	if($gprs != null) 	mysqli_query($dbc, "INSERT INTO ".$sr."_gprs(gprs) VALUES ('$err')");
	if($cne != null)	mysqli_query($dbc, "INSERT INTO ".$sr."_cne(cne) VALUES ('$cne')");
	if($b == $st['b'])	mysqli_query($dbc, "UPDATE ".$sr."_status SET n=0 WHERE id=1");
	if($cnn == $st['cnn']) {
		if($cnn == 0)	$cnnRs = 1;
		else 			$cnnRs = 0;
		mysqli_query($dbc, "UPDATE ".$sr."_status SET cnn='$cnnRs' WHERE id=1");
	}
	else
		$cnnRs = $st['cnn'];
	
	if($h != null) {
		$temp = hexdec(substr($h,0,4));
		$volt = hexdec(substr($h,4,4));
		$temp = round(abs($temp*95/4095-95),2);
		$volt = round($volt*51/4095,2);
		mysqli_query($dbc, "UPDATE ".$sr."_status SET temp='$temp',volt='$volt' WHERE id=1");
		if($volt < 5) {
			mysqli_query($dbc, "INSERT INTO ".$sr."_pin(pin) VALUES ('1')");
		}
	}


	if($o1 != null) {
		$ar = str_split($o1);
		$rs1 = $ar[0];
		$rs2 = $ar[1];
		$rs3 = $ar[2];
		$rs4 = $ar[3];
		mysqli_query($dbc, "UPDATE ".$sr."_status SET o1='$o1', rs1='$rs1', rs2='$rs2', rs3='$rs3', rs4='$rs4' WHERE id=1");
	}

	if($i2 != null) {
		$ar = array();
		$temp = $i2;
		for($i=0; $i<8; $i++) {
			$ar[$i] = $temp & 0x01;
			$temp = $temp >> 1;
		}
		$i20 = $ar[0];
		$i21 = $ar[1];
		$i22 = $ar[2];
		$i23 = $ar[3];
		$i24 = $ar[4];
		$i25 = $ar[5];
		$i26 = $ar[6];
		$i27 = $ar[7];

		mysqli_query($dbc, "UPDATE ".$sr."_status SET i2='$i2', i20='$i20', i21='$i21', i22='$i22', i23='$i23', i24='$i24', i25='$i25', i26='$i26', i27='$i27' WHERE id=1");
	}


	

	$esp = mysqli_fetch_array( mysqli_query($dbc, "SELECT * FROM ".$sr."_status WHERE id=1") );
	echo $cnnRs.$esp['n'].$esp['d'].$esp['b'].",".$esp['r1'].$esp['r2'].$esp['r3'].$esp['r4'];

	if($m == 1 )
        $m = 50;                        
    else if($m == 2)
        $m = 50;
    else if($m == 3)
        $m = 75;
    else if($m == 4)         
    	$m = 100;
	if($sr!= null){
		$query = "UPDATE ".$sr."_status SET s1 ='{$s1}',s2 ='{$s2}',s3 ='{$s3}',s4 ='{$s4}',level ='{$m}' WHERE id=1";
		if (!mysqli_query($dbc, $query)) {
		mysqli_error($dbc);   
		}
		if($dr != null) {
			$qr = "INSERT INTO daily_rst(dr) VALUES ('$dr')";
			if (!mysqli_query($dbc, $qr)) {
				mysqli_error($dbc);   
	    	}   
		} 
    }            			




		//echo $sr.$s1.$s2.$m;		
	if($s1==1){	
		bat($sr,1);
    }
   	if($s1==0){
		tat($sr,1);	
    }
    if($s2==1){	
    	bat($sr,2);  
    }
    if($s2==0){
		tat($sr,2);			
   	}
	if($s3==1){	
		bat($sr,3);  	
    }
    if($s3==0){
		tat($sr,3);		
   	}
	mysqli_close($dbc);
		
}

function bat($sr,$bom)
{	include('inc/myconnect.php');
	$queri="SELECT tat,bat,rst,ngaygio FROM ".$sr."_baocao_s".$bom." ORDER BY id DESC LIMIT 1";
	$result=mysqli_query($dbc,$queri);			
	while($row = mysqli_fetch_array($result,MYSQLI_ASSOC)) 
		{	$off=$row['tat']; $on = $row['bat'];$rst = $row['rst'];	$ngaygio=$row['ngaygio'];}
	if($off != "0000-00-00 00:00:00" || $on == "0000-00-00 00:00:00" || $on == null){		
		$gio=date("Y-m-d H:i:s");			
      	$bat="INSERT INTO ".$sr."_baocao_s".$bom."(bat) VALUES ('$gio')";
		mysqli_query($dbc,$bat); 
	}
	else if($on != "0000-00-00 00:00:00" && $on != null && $rst==0){
		$ht=date("Y-m-d H:i:s");
		$diff= abs(strtotime($ht) - strtotime($on));
		$years = floor($diff / (31536000));
		$months = floor(($diff - $years * 31536000) / (2592000));
		$days = floor(($diff - $years * 31536000 - $months*2592000) / (86400));
		$hours = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400) / (3600));
		$minutes = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400 - $hours*3600) / 60);
		$seconds = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400 - $hours*3600 - $minutes*60));
		$dateint = mktime($hours,$minutes,$seconds);
		$thoigian = "0000-".$months."-".$days.date(' H:i:s',$dateint);
		$tat="UPDATE ".$sr."_baocao_s".$bom." SET thoigian='{$thoigian}' ORDER BY id DESC LIMIT 1";
		$res=mysqli_query($dbc,$tat);
		$diff= abs(strtotime($ht) - strtotime($ngaygio));
		$hours = floor(($diff) / (3600));
		$minutes = floor(($diff - $hours*3600) / 60);
		$seconds = floor(($diff - $hours*3600 - $minutes*60));
		if( $hours >= 1 || $minutes >= 1){
			$tat="UPDATE ".$sr."_baocao_s".$bom." SET ngaygio=NOW() ORDER BY id DESC LIMIT 1";
			mysqli_query($dbc,$tat);
			$tb="tong".$bom; $cb="gio".$bom; $d="dem".$bom;
			$sql="SELECT ".$tb.",".$cb.",".$d." FROM ".$sr."_status WHERE id=1";
			$result_min=mysqli_query($dbc,$sql);			
			while($row = mysqli_fetch_array($result_min,MYSQLI_ASSOC)) 
				{ $tong=$row[$tb]; $gio=(float)$row[$cb]; $dem=(float)$row[$d]; }
		 	$tong =  (float)$hours +  (float)$minutes/60 +  (float)$seconds/3600 + (float)$tong;		 	
		 	$tong=round($tong, 2);
		 	$ll = $tong*130; 
		 	$o=2;
			if($gio != 0){
				if($tong >= $gio && $dem == 0){
					$sql="SELECT toi FROM gmail WHERE id=1";
					$result_min=mysqli_query($dbc,$sql);			
					while($row = mysqli_fetch_array($result_min,MYSQLI_ASSOC)) 
						{ $toi=$row['toi']; }
					$url="http://".$_SERVER["HTTP_HOST"]."/sendgrid/send-email.php?sr=".$sr."&tong=".$tong."&bom=".$bom."&toi=".$toi."&gio=".$gio;
	                $ch = curl_init();
	                curl_setopt($ch, CURLOPT_URL, $url);
	                curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
	                curl_exec($ch); 
	                curl_close($ch);
	                $o=1;               
				}elseif ($tong < $gio) {
					$o=0;
				}
			}
			$tat="UPDATE ".$sr."_status SET ".$tb."='{$tong}',flow".$bom."='{$ll}',".$d."=".$o." WHERE id=1";
			$res=mysqli_query($dbc,$tat);
		}
	}
}
function tat($sr,$bom)
{
	include('inc/myconnect.php');
		$date2=date("Y-m-d H:i:s");
		$sql_min="SELECT bat,total,rst,ngaygio FROM ".$sr."_baocao_s".$bom." ORDER BY id DESC LIMIT 2";
			$result_min=mysqli_query($dbc,$sql_min);
		$dem=0;					
		while($row = mysqli_fetch_array($result_min,MYSQLI_ASSOC)) 
			{   
				$dem++;
				if($dem==1){
			  	   	$date1=$row['bat'];$rst=$row['rst'];$ngay1=$row['ngaygio'];
			  	   	// $ngaygio2=
				}
				$ngay2=$row['ngaygio'];
			  	$total_c=date("H:i:s", strtotime($row['total']));
			}	
	
		if($date1 != "0000-00-00 00:00:00" && $date1 != null && $rst !=1){ 
			$tat="UPDATE ".$sr."_status SET dem".$bom."=0 WHERE id=1";
			$res=mysqli_query($dbc,$tat);
			$adif = abs(strtotime($date2) - strtotime($date1) + strtotime($total_c)) ;
			$diff= abs(strtotime($date2) - strtotime($date1));
			$years = floor($diff / (31536000));
			$months = floor(($diff - $years * 31536000) / (2592000));
			$days = floor(($diff - $years * 31536000 - $months*2592000) / (86400));
			$hours = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400) / (3600));
			$minutes = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400 - $hours*3600) / 60);
			$seconds = floor(($diff - $years * 31536000 - $months*2592000 - $days*86400 - $hours*3600 - $minutes*60));

			$dateint = mktime($hours,$minutes,$seconds);
			$thoigian = "0000-".$months."-".$days.date(' H:i:s',$dateint);
			$total_day = date("d",$adif);$tach_day=date("d");
			$tru_ngay = $total_day - $tach_day ;
		 	$total = "0000-00-".$tru_ngay.date(' H:i:s',$adif);
		 // 	$tb="tong".$bom;
		 // 	$sql="SELECT ".$tb." FROM ".$sr."_status WHERE id=1";
			// $result_min=mysqli_query($dbc,$sql);			
			// while($row = mysqli_fetch_array($result_min,MYSQLI_ASSOC)) 
			// 	{ $tong=$row[$tb]; }
		 // 	$tong =  (float)$hours +  (float)$minutes/60 +  (float)$seconds/3600 + (float)$tong;		 	
		 // 	$tong=round($tong, 2);
		 // 	$ll = $tong*130; 
		 // 	$tat="UPDATE ".$sr."_status SET tong".$bom."='{$tong}',flow".$bom."='{$ll}' WHERE id=1";
			// $res=mysqli_query($dbc,$tat);
		 	if($total_c == "0000-00-00 00:00:00" || date('d-m-Y', strtotime($ngay1))!=date('d-m-Y', strtotime($ngay2))){ $total = $thoigian ;}
		 	// if(date('d-m-Y', strtotime($bat))==date('d-m-Y', strtotime($tat)))
		 	$query="SELECT COUNT(id) FROM ".$sr."_baocao_s".$bom." WHERE (DATE(ngaygio) = DATE(NOW()))";
	        $result = mysqli_query($dbc, $query);
	        $get = mysqli_fetch_assoc($result);
	        $solan = $get['COUNT(id)']; 
	        if($solan != null ){
	        	$query="SELECT solan FROM ".$sr."_report_s".$bom." WHERE (DATE(ngaygio) = DATE(NOW()))";
		        $result = mysqli_query($dbc, $query);
		        $get = mysqli_fetch_assoc($result);
		        $tach = getdate(strtotime($total));		        				
   				$s = ($tru_ngay*24 + $tach[hours] + $tach[minutes]/60 + $tach[seconds]/3600)*130 ;
   				
   				$flow = round($s,2);		       
		        if($get['solan'] == null){
		        	$total = $thoigian ;
		        	$tach = getdate(strtotime($total));		        				
	   				$s = ($tach[hours]+ $tach[minutes]/60 + $tach[seconds]/3600)*130 ;
	   				$flow = round($s,2);
		        	$bat_in="INSERT INTO ".$sr."_report_s".$bom."(solan,total,flow) VALUES (1,'{$total}','{$flow}')";
					$result_in=mysqli_query($dbc,$bat_in);
					if($result_in){	
						$tat_in="UPDATE ".$sr."_baocao_s".$bom." SET tat='{$date2}',thoigian='{$thoigian}',total='{$total}',rst=1 ORDER BY id DESC LIMIT 1";
					 	$result_in=mysqli_query($dbc,$tat_in);	       				
					}
		        }else{
		        	$tat_in="UPDATE ".$sr."_report_s".$bom." SET solan='{$solan}',total='{$total}',flow='{$flow}' ORDER BY id DESC LIMIT 1";
				 	mysqli_query($dbc,$tat_in);	       
						        	        
			        $tat_in="UPDATE ".$sr."_baocao_s".$bom." SET tat='{$date2}',thoigian='{$thoigian}',total='{$total}',rst=1 ORDER BY id DESC LIMIT 1";
				 	mysqli_query($dbc,$tat_in);	       				
				}	
			}
		}
}		
?>
