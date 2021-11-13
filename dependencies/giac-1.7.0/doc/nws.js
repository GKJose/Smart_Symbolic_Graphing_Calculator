function $id(id) { return document.getElementById(id); }
var UI = {
  chkmsgfr:'Verifiez que la calculatrice Numworks est connectee.\nCliquez sur le bouton Detecter puis reessayez.',
  chkmsgen:'Check that the Numworks calculator is connected.\nClick on the detect button and try again.',
  nws_progress:0,
  nws_progresslegend:0,
  Datestart:Date.now(),
  langue: -1, // -1 french
  calc: 2, // 1 KhiCAS, 2 Numworks, 3 TI Nspire CX
  calculator:0, // !=0 if hardware Numworks connected
  calculator_connected:false,
  tar_first_modif_offset:0,
  nws_connect:function(){
    UI.nws_progress=$id('plog');
    UI.nws_progresslegend=$id('ploglegend');
    if (navigator.usb){
      //console.log('nws_connect 0');
      UI.calculator=0;
      UI.calculator_connected=false;
      //console.log('nws_connect 1');
      function autoConnectHandler(e) {
	UI.calculator.stopAutoConnect();
	console.log('connected');
	UI.calculator_connected=true;
      }
      UI.calculator= new Numworks();
      console.log('nws_connect',UI.calculator);
      navigator.usb.addEventListener("disconnect", function(e) {
	if (UI.calculator==0) return;
	UI.calculator.onUnexpectedDisconnect(e, function() {
	  UI.calculator_connected=false;
          UI.calculator=0;
	});
      });
      //console.log('nws_connect 2');
      UI.calculator.autoConnect(autoConnectHandler);
      return UI.calculator;
      //console.log('nws_connect 3');
    }
  },
  nws_detect_success:function(){
    alert('Success');
  },
  nws_detect_failure:function(error){
    console.log(error);
    alert('Failure');
  },
  nws_detect:async function(success,failure){
    UI.calculator= new Numworks();
    UI.calculator.detect(success,failure);
  },
  nws_rescue_connect:async function(){
    if (navigator.usb){
      //console.log('nws_rescue_connect 0');
      UI.calculator=0;
      UI.calculator_connected=false;      //console.log('nws_rescue_connect 1');
      function autoConnectHandler(e) {
	UI.calculator.stopAutoConnect();
	console.log('rescue connected');
	UI.calculator_connected=true;
      }
      UI.calculator= new Numworks.Recovery();
      console.log('nws_rescue_connect',UI.calculator);
      navigator.usb.addEventListener("rescue disconnect", function(e) {
	if (UI.calculator==0) return;
	UI.calculator.onUnexpectedDisconnect(e, function() {
	  UI.calculator_connected=false;
          UI.calculator=0;
	});
      });
      //console.log('nws_rescue_connect 2');
      if (await UI.calculator.autoConnect(autoConnectHandler)==-1){
	return -1;
      }
      //console.log('nws_rescue_connect 3');
      return 0;
    }
  },
  sig_check:async function(sig,data,fname){
    // sig should be a list of lists of size 3 (name, length, hash)
    let i=0,l=sig.length;
    for (;i<l;++i){
      let cur=sig[i];
      if (cur[0]!=fname) continue;
      console.log('sig_check',i,cur[1],data.byteLength);
      if (cur[1]>data.byteLength) continue;
      let dat=data.slice(0,cur[1]);
      let digest = await window.crypto.subtle.digest('SHA-256', dat);
      digest=Array.from(new Uint8Array(digest));
      console.log(cur[2],digest);
      let j=0;
      for (;j<32;++j){
	let tst=(digest[j]-cur[2][j]) % 256;
	// console.log(j,digest[j],cur[2][j]);
	if (tst)
	  break;
      }
      if (j==32){
	console.log('signature match',cur[0]);
	return true;
      }      
    }
    return false;
  },
  numworks_load:function(backup=false){
    UI.calc=2;
    UI.nws_connect();
    window.setTimeout(UI.numworks_load_,100,backup);
  },
  numworks_load_: async function(backup){
    console.log(UI.calculator,UI.calculator_connected);
    if (UI.calculator==0 || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return;
    }
    if (backup){
      let pinfo = await UI.calculator.getPlatformInfo();
        
      let storage_blob = await UI.calculator.__retreiveStorage(pinfo["storage"]["address"], pinfo["storage"]["size"]);
      filename = "backup.nws";
      saveAs(storage_blob.slice(0,32768), filename);
      return;
    }
    let storage = await UI.calculator.backupStorage();
    let rec=UI.storage_records=storage.records;
    console.log(UI.store2html(rec,'storelist'));
    return;
    let j=0,s='Choisissez un numero parmi ';
    for (;j<rec.length;++j){
      s+=j;
      s+=':'+rec[j].name+', ';
    }
    let p=prompt(s),n=0;
    if (!p) return;
    let l=p.length;
    for (j=0;j<l;++j){
      if (p[j]<'0' || p[j]>'9'){ alert(UI.langue==-1?'Nombre invalide':'Invalid number'); return ;}
      n*=10;
      n+=p.charCodeAt(j)-48;
    }
    if (n>=rec.length){ alert(UI.langue==-1?'Choix invalide':'Invalide choice'); return; }
    s=rec[n].code; p=rec[n].name;
    let blob = new Blob([s], {type: "text/plain;charset=utf-8"});
    filename = p + ".py";
    saveAs(blob, filename);
  },
  loadFile:function(file){
    let reader = new FileReader();
    reader.onerror = function (evt) { }
    if (file.name.length>4 && file.name.substr(file.name.length-4,4)==".nws"){
      if (!confirm(UI.langue==-1?'Remplacer tous les scripts de la calculatrice?':'Overwrite all calculator scripts?'))
	return;
      reader.readAsArrayBuffer(file);
    }
    else
      reader.readAsText(file, "UTF-8");
    reader.onload = function (evt) { UI.numworks_save(file.name,evt.target.result);}
  },
  numworks_save:function(filename,S){
    //console.log(filename,S); return;
    UI.nws_connect();
    window.setTimeout(UI.numworks_save_,100,filename,S);
  },
  numworks_save_:async function(filename,S){
    if (UI.calculator==0 || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return -1;
    }
    if (filename.length>4 && filename.substr(filename.length-4,4)==".nws"){
      let pinfo = await UI.calculator.getPlatformInfo();
      UI.calculator.device.startAddress = pinfo["storage"]["address"];
      // console.log(UI.calculator.device.startAddress,S); // return;
      let res=await UI.calculator.device.do_download(UI.calculator.transferSize, S, false);
      return res;
    }
    if (filename.length>3 && filename.substr(filename.length-3,3)==".py")
      filename=filename.substr(0,filename.length-3);
    let storage = await UI.calculator.backupStorage();
    let rec=storage.records,j=0;
    for (;j<rec.length;++j){
      if (rec[j].name==filename){
	if (!confirm((UI.langue==-1?'? Ecraser ':'? Overwrite ')+rec[j].name))
	  return;
	rec[j].code=S;
	break;
      }
    }
    if (j==rec.length)
      rec.push({"name": filename, "type":"py", "autoImport": false, "code": S});
    await UI.calculator.installStorage(storage, function() {
      // Do stuff after writing to the storage is done
      console.log(filename+'.py saved to Numworks');
    });
    return 0;
  },    
  numworks_install_delta:function(do_backup=1,khionly=0){
    if (khionly && !confirm(UI.langue==-1?'En confirmant l\'installation, vous acceptez la licence CC BY-NC-SA de Khi':'If you confirm, you accept the CC BY-NC-SA license of Khi.'))
      return;
    if (!khionly && !confirm(UI.langue==-1?'En confirmant l\'installation, vous acceptez les licences CC BY-NC-SA de Khi et GPL2 de KhiCAS, BSD2 Hexedit, LGPL2 de Nofrendo, MIT de Peanut-GB':'If you confirm install, you accept the CC BY-NC-SA license of Khi and GPL2 license of KhiCAS, BSD2 for Hexedit, LGPL2 for Nofrendi, MIT for Peanut-GB'))
      return;
    $id('persoinit').style.display='none';    
    UI.calc=2;
    UI.nws_connect();
    window.setTimeout(UI.numworks_install_delta_,100,do_backup,khionly);
  },
  numworks_install_delta_:async function(do_backup,khionly){
    if (UI.calculator==0 || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return -1;
    } 
    UI.print(khionly?'=========== installing Khi':'=========== installing Khi and KhiCAS');
    UI.print('erase/write Khi external, wait about 1/2 minute');
    UI.calculator.device.startAddress = 0x90000000;
    let data=await UI.loadfile('delta.external.bin');
    UI.calculator.device.logProgress(0,'external');
    let res=await UI.calculator.device.do_download(UI.calculator.transferSize, data, false);
    UI.print('Khi external OK, erase/write internal');
    UI.calculator.device.startAddress = 0x08000000;
    UI.calculator.device.logProgress(0,'internal');
    data=await UI.loadfile('delta.internal.bin');
    res=await UI.calculator.device.do_download(UI.calculator.transferSize, data, false);
    if (khionly){
      UI.print('Khi internal OK, press RESET on calculator back');
      alert(UI.langue==-1?'Installation terminee. Appuyer sur RESET a l\'arriere de la calculatrice':'Install success. Press RESET on the calculator back.');
      return;
    }
    UI.print('Khi internal OK, erase/write KhiCAS and apps, wait about 2 minutes');
    UI.calculator.device.startAddress = 0x90200000;
    UI.calculator.device.logProgress(0,'apps');
    data=await UI.loadfile('apps.tar');
    res=await UI.calculator.device.do_download(UI.calculator.transferSize, data, false);
    UI.print('apps OK, press RESET on calculator back');
    alert(UI.langue==-1?'Installation terminee. Appuyer sur RESET a l\'arriere de la calculatrice':'Install success. Press RESET on the calculator back.');
    return 0;
  },
  loadfile:function(file) {
    return fetch(file).then(function(response) {
      if (!response.ok) {
        throw new Error("HTTP error, status = " + response.status);
      }
      return response.arrayBuffer();
    })
  },
  numworks_rescue:async function(){
    alert(UI.langue==-1?'Connectez la calculatrice, appuyez sur la touche 6 de la calculatrice, enfoncez un stylo dans le bouton RESET au dos en laissant la touche 6 appuyee, relachez la touche 6, l\'ecran doit etre eteint et la diode rouge allumee':'Connect the calculator, press the 6 key on the calculator, press the RESET button on the back keeping the 6 key pressed, release the 6 key, the screen should be down and the led should be red');
    UI.calc=2;
    let res=await UI.nws_rescue_connect();
    if (res==-1){
      return;
    }
    window.setTimeout(UI.numworks_rescue_,1000);
  },
  numworks_rescue_:async function(sigfile,rwcheck){
    if (UI.calculator==0 || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      //console.log('numworks_rescue_',UI.calculator);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      UI.calculator=0;
      return -1;
    }
    UI.print(UI.langue==-1?'Envoi du mode de recuperation, patientez environ 15 secondes':'Sending rescue mode, wait about 15 secondes');
    UI.calculator.device.startAddress = 0x20030000;
    let data=await UI.loadfile('recovery');
    await UI.calculator.device.clearStatus();
    await UI.calculator.device.do_download(UI.calculator.transferSize, data, true);
    let tmp=confirm(UI.langue==-1?'La calculatrice devrait etre en mode recuperation. Cliquer sur le bouton Detecter puis sur install KhiCAS':'Calculator should be in rescue mode. Click on the Detect button then install KhiCAS.');
    return;
    if (tmp)
      UI.nws_detect(UI.numworks_install_delta,UI.nws_detect_failure);
    else
      UI.nws_detect(UI.numworks_detect_success,UI.nws_detect_failure);
  },
  numworks_buffer:0,
  read_string:function(buffer,str_offset, size) {
    let strView = new Uint8Array(buffer, str_offset, size);
    let i = 0;
    let rtnStr = "";
    while(strView[i] != 0) {
      rtnStr += String.fromCharCode(strView[i]);
      i++;
    }
    return rtnStr;
  },
  // tar support adapted from tarball.js https://github.com/ankitrohatgi/tarballjs (MIT licence)
  filename(buffer,header_offset) {
    let name = UI.read_string(buffer,header_offset, 100);
    return name;
  },
  filetype(buffer,header_offset) {
    // offset: 156
    let typeView = new Uint8Array(buffer, header_offset+156, 1);
    let typeStr = String.fromCharCode(typeView[0]);
    if(typeStr == "0") {
      return "file";
    } else if(typeStr == "5") {
      return "directory";
    } else {
      return typeStr;
    }
  },
  filesize(buffer,header_offset) {
    // offset: 124
    let szView = new Uint8Array(buffer, header_offset+124, 12);
    let szStr = "";
    for(let i = 0; i < 11; i++) {
      let tmp=szView[i];
      if (tmp<48 || tmp>57) return -1; // invalid file size
      szStr += String.fromCharCode(tmp);
    }
    return parseInt(szStr,8);
  },
  numworks_chk_buffer:function(buffer){
    if (buffer==0){
      alert(UI.langue==-1?'Commencez par recuperer la flash de la calculatrice':'First get the calculator flash');
      return 0;
    }
    // FIXME should check that buffer is valid
    return 1;
  },
  numworks_tarinfo:function(buffer){
    if (!UI.numworks_chk_buffer(buffer))
      return [];
    let offset = 0;
    let file_size = 0;       
    let file_name = "";
    let file_type = null;
    let file_info=[];
    while(offset < buffer.byteLength - 512) {
      file_name = UI.filename(buffer,offset); // file name
      if (file_name.length == 0) break;
      file_type = UI.filetype(buffer,offset);
      file_size = UI.filesize(buffer,offset);
      if (file_size<0) break;
      file_info.push({
        "name": file_name,
        "type": file_type,
        "size": file_size,
        "header_offset": offset
      });
      offset += (512 + 512*Math.trunc(file_size/512));
      if(file_size % 512) {
        offset += 512;
      }
    }
    return file_info;
  },
  leftpad:function(number,targetLength) {
    let output = number + '';
    while (output.length < targetLength) {
      output = '0' + output;
    }
    return output;
  },
  numworks_maxtarsize:0x600000-0x10000,
  arrayBufferTransfer:function(oldBuffer, newByteLength) {
    const srcArray  = new Uint8Array(oldBuffer),
          destArray = new Uint8Array(newByteLength),
          copylen = Math.min(srcArray.buffer.byteLength, destArray.buffer.byteLength),
          floatArrayLength   = Math.trunc(copylen / 8),
          floatArraySource   = new Float64Array(srcArray.buffer,0,floatArrayLength),
          floarArrayDest     = new Float64Array(destArray.buffer,0,floatArrayLength);
    floarArrayDest.set(floatArraySource);
    let bytesCopied = floatArrayLength * 8;
    // slowpoke copy up to 7 bytes.
    while (bytesCopied < copylen ) {
      destArray[bytesCopied]=srcArray[bytesCopied];
      bytesCopied++;
    }
    return destArray.buffer;
  },
  tar_filesize:function(s){
    return 512*(1+Math.ceil(s/512));
  },
  _readString:function(buffer,str_offset, size) {
    let strView = new Uint8Array(buffer, str_offset, size);
    let i = 0;
    let rtnStr = "";
    while(i<size) {
      let ch=strView[i];
      if (ch<32) break;
      rtnStr += String.fromCharCode(ch);
      i++;
    }
    return rtnStr;
  },
  _readFileName:function(buffer,header_offset) {
    return UI._readString(buffer,header_offset, 100);
  },
  _readFileType:function(buffer,header_offset) {
    // offset: 156
    let typeView = new Uint8Array(buffer, header_offset+156, 1);
    let typeStr = String.fromCharCode(typeView[0]);
    if(typeStr == "0") {
      return "file";
    } else if(typeStr == "5") {
      return "directory";
    } else {
      return typeStr;
    }
  },
  _readFileSize:function(buffer,header_offset) {
    // offset: 124
    let szView = new Uint8Array(buffer, header_offset+124, 12);
    let szStr = "";
    for(let i = 0; i < 11; i++) {
      let tmp=szView[i];
      if (tmp<48 || tmp>57) return -1; // invalid file size
      szStr += String.fromCharCode(tmp);
    }
    return parseInt(szStr,8);
  },
  tar_clear:function(buffer){
    let view = new Uint8Array(buffer);
    for (let i=0;i<1024;++i)
      view[i]=0;
    $id('listtarfiles').innerHTML="";
    $id('perso').style.display='inline';    
  },
  tar_fileinfo:function(buffer,dohtml=1){
    let fileInfo = [];
    let offset = 0;
    let file_size = 0;       
    let file_name = "";
    let file_type = null;
    while (offset < buffer.byteLength - 512) {
      file_name = UI._readFileName(buffer,offset); // file name
      if (file_name.length == 0) 
        break;
      file_type = UI._readFileType(buffer,offset);
      file_size = UI._readFileSize(buffer,offset);
      if (file_size<0)
	break;
      //console.log(offset,file_name,file_size);
      fileInfo.push({
        "name": file_name,
        "type": file_type,
        "size": file_size,
        "header_offset": offset
      });

      offset += UI.tar_filesize(file_size);
    }
    //console.log('fileinfo',offset,dohtml);
    if (dohtml) UI.fileinfo2html(fileInfo,'listtarfiles');
    return fileInfo;
  },
  fileinfo2html:function(finfo,fieldname){
    let l=finfo.length,s="<ul>";
    for (let i=0;i<l;++i){
      let cur=finfo[i];
      let S="<li>";
      S += cur.name;
      S += " (";
      S += cur.size;
      S += "): ";
      S += "<button onclick='UI.tar_removefile(UI.numworks_buffer,";
      S += '"'+cur.name+'"';
      S +=");' title='Cliquer ici pour effacer ce fichier'>X</button>";
      S += "<button onclick='UI.tar_savefile(";
      S += '"'+cur.name+'"';
      S +=");' title='Cliquer ici pour sauvegarder ce fichier'>&#x1f4be;</button>";      
      s+=S;
    }
    s += "</ul>";
    //console.log(s);
    // put it the list of tarfiles
    if (fieldname){
      let field=$id(fieldname);
      field.innerHTML=s;
      field.style.display='inline';
      field=field.parentNode.style.display='inline';
    }
    // $id('perso').style.display='inline';
    return s;
  },
  storage_records:[],
  store_savefile:function(filename){
    let rec,j=0;
    for (j=0;j<UI.storage_records.length;++j){
      rec=UI.storage_records[j];
      if (rec.name==filename) break;
    }
    if (j==UI.storage_records.length) return;
    let blob = new Blob([rec.code], {type: "text/plain;charset=utf-8"});
    if (filename.length<=3 || filename.substr(filename.length-3,3)!=".py")
      filename +=  ".py";
    saveAs(blob, filename);
  },
  store2html:function(finfo,fieldname){
    let l=finfo.length,s="<ul>";
    for (let i=0;i<l;++i){
      let cur=finfo[i];
      let S="<li>";
      S += cur.name;
      S += " (";
      S += cur.size;
      S += "): ";
      //S += "<button onclick='UI.store_removefile(UI.numworks_buffer,";
      //S += '"'+cur.name+'"';
      //S +=");' title='Cliquer ici pour effacer ce fichier'>X</button>";
      S += "<button onclick='UI.store_savefile(";
      S += '"'+cur.name+'"';
      S +=");' title='Cliquer ici pour sauvegarder ce fichier'>&#x1f4be;</button>";      
      s+=S;
    }
    s += "</ul>";
    //console.log(s);
    // put it the list of tarfiles
    if (fieldname){
      s += '<button onclick="$id(\''+fieldname+'\').style.display=\'none\'">Fermer</button>';
      let field=$id(fieldname);
      field.innerHTML=s;
      field.style.display='inline';
      return s;
    }
    // $id('perso').style.display='inline';
    return s;
  },
  tar_writestring:function(buffer,str, offset, size) {
    let strView = new Uint8Array(buffer, offset, size);
    for(let i = 0; i < size; i++) {
      if (i < str.length) {
        strView[i] = str.charCodeAt(i);
      } else {
        strView[i] = 0;
      }
    }
  },
  tar_writechecksum:function(buffer,header_offset) {
    // offset: 148
    UI.tar_writestring(buffer,"        ", header_offset+148, 8); // first fill with spaces

    // add up header bytes
    let header = new Uint8Array(buffer, header_offset, 512);
    let chksum = 0;
    for (let i = 0; i < 512; i++) {
      chksum += header[i];
    }
    UI.tar_writestring(buffer,UI.leftpad(chksum.toString(8),6), header_offset+148, 8);
    UI.tar_writestring(buffer," ",header_offset+155,1); // add space inside chksum field
  },
  tar_fillheader:function(buffer,offset,exec=0){
    let uid = 501;
    let gid = 20;
    let mode = exec?"755":"644";
    let mtime = Date.now();
    let user = "user";
    let group = "group";

    UI.tar_writestring(buffer,UI.leftpad(mode,6)+" ", offset+100, 8);  
    UI.tar_writestring(buffer,UI.leftpad(uid.toString(8),6)+" ",offset+108,8);
    UI.tar_writestring(buffer,UI.leftpad(gid.toString(8),6)+" ",offset+116,8);
    UI.tar_writestring(buffer,UI.leftpad(Math.trunc(mtime/1000).toString(8),11)+" ",offset+136,12);

    //UI.tar_writestring(buffer,"ustar", offset+257,6); // magic string
    //UI.tar_writestring(buffer,"00", offset+263,2); // magic version
    UI.tar_writestring(buffer,"ustar  ", offset+257,8);
    
    UI.tar_writestring(buffer,user, offset+265,32); // user
    UI.tar_writestring(buffer,group, offset+297,32); //group
    UI.tar_writestring(buffer,"000000 ",offset+329,7); //devmajor
    UI.tar_writestring(buffer,"000000 ",offset+337,7); //devmajor
    UI.tar_writechecksum(buffer,offset);
  },
  tar_adddata:function(buffer,filename,data,exec=0){
    console.log('tar_adddata',exec);
    let finfo=UI.tar_fileinfo(buffer,0);
    let s=finfo.length;
    if (s==0) return 0;
    let last=finfo[s-1];
    let offset=last.header_offset;
    offset += UI.tar_filesize(last.size);
    let newsize=offset+1024+data.byteLength;
    newsize=10240*Math.ceil(newsize/10240);
    if (newsize>UI.numworks_maxtarsize) return 0;
    // console.log(buffer.byteLength,newsize);
    // resize buffer
    if (buffer.byteLength<newsize)
      buffer=UI.numworks_buffer=UI.arrayBufferTransfer(buffer,newsize);
    // console.log(buffer.byteLength,newsize);
    let view = new Uint8Array(buffer);    
    // add header
    // fill header with 0
    for (let i=0;i<1024;++i)
      view[offset+i]=0;
    UI.tar_writestring(buffer,filename,offset,100); // filename
    UI.tar_writestring(buffer,UI.leftpad(data.byteLength.toString(8),11)+" ",offset+124,12);  // filesize
    UI.tar_writestring(buffer,"0",offset+156,1); // file type
    UI.tar_fillheader(buffer,offset,exec);
    //let tmp=new Uint8Array(buffer,offset); console.log(tmp);
    //console.log(data.byteLength);
    // copy data 
    let srcview = new Uint8Array(data, 0, data.byteLength);    
    for (let i=0;i<data.byteLength;++i)
      view[offset+512+i]=srcview[i];
    let F=UI.tar_fileinfo(buffer,1); //console.log(F);
    return 1;
  },
  tar_removefile:function(buffer,filename,really=1){
    let finfo=UI.tar_fileinfo(buffer,0);
    let s=finfo.length;
    if (s==0) return 0;
    let i = finfo.findIndex(info => info.name == filename);
    if (i<0 || i>=s) return 0;
    if (really && !confirm(UI.langue==-1?'Etes-vous sur?':'Are you sure?'))
      return 0;
    let info = finfo[i];
    let view = new Uint8Array(buffer);
    // move info.header_offset+info.size+512 to info.header_offset
    let target=info.header_offset;
    if (target<UI.tar_first_modif_offset){
      UI.tar_first_modif_offset=target;
      console.log('tar_removefile',UI.tar_first_modif_offset);
    }
    let src=target+UI.tar_filesize(info.size);
    let infoend=finfo[s-1];
    let end=infoend.header_offset+UI.tar_filesize(infoend.size);
    for (src;src<end;++src,++target)
      view[target]=view[src];
    for (;target<end;++target) // clear space after new end
      view[target]=0;
    return UI.tar_fileinfo(buffer,1);
  },
  tar_addfile:function(file){
    let reader = new FileReader();
    reader.onerror = function (evt) { }
    reader.readAsArrayBuffer(file);
    reader.onload = function(evt){
      let fname=file.name,exec=1;
      if (fname.length>4 && fname.substr(fname.length-4,4)==".zip"){
	let zip=new JSZip();
	zip.loadAsync(evt.target.result).then(
	  function (zip) {
	    let lst=Object.entries(zip.files);
	    console.log(zip.files,lst);
	    for (let i=0;i<lst.length;++i){
	      let cur=lst[i];
	      let curname=cur[1].name;
	      console.log(curname);
	      zip.file(curname).async("arraybuffer").then(
		function(data){
		  exec=1;
		  for (let j=0;j<curname.length;++j){
		    if (curname[j]=='.'){
		      exec=0; break;
		    }
		  }
		  if (UI.tar_adddata(UI.numworks_buffer,curname,data,exec)==0){
		    alert(UI.langue==-1?'Pas d\'espace en memoire flash':'No space left in flash');
		    return;
		  }
		}
	      );
	    }
	  }
	);
	return;
      }
      for (let i=0;i<fname.length;++i){
	if (fname[i]=='.'){
	  exec=0; break;
	}
      }
      console.log(fname,exec);
      if (UI.tar_adddata(UI.numworks_buffer,file.name,evt.target.result,exec)==0)
	alert(UI.langue==-1?'Pas d\'espace en memoire flash':'No space left in flash');
    }
  },
  file_gettar:function(file){
    console.log(file);
    if (UI.numworks_buffer!=0){
      if (!confirm('Attention, la personnalisation courante va etre effacee! Continuer?'))
	return;
    }
    let reader = new FileReader();
    reader.onerror = function (evt) { }
    reader.readAsArrayBuffer(file);
    reader.onload = function(evt){
      UI.tar_first_modif_offset=0;
      UI.numworks_buffer=evt.target.result;
      console.log(UI.tar_fileinfo(UI.numworks_buffer,1));
    }
  },
  file_savetar:function(buffer,filename){
    let finfo=UI.tar_fileinfo(buffer,0);
    let s=finfo.length;
    if (s==0) return 0;
    let infoend=finfo[s-1];
    let end=infoend.header_offset+UI.tar_filesize(infoend.size);
    // add at least 1024 bytes, set them to 0
    let newsize=end+1024;  //newsize=10240*Math.ceil(newsize/10240);
    if (buffer.byteLength<newsize)
      buffer=UI.numworks_buffer=UI.arrayBufferTransfer(buffer,newsize);
    let view=new Uint8Array(buffer,0,newsize); 
    for (let i=end;i<newsize;++i)
      view[i]=0;
    let blob = new Blob([view], {type: "octet/stream"});
    saveAs(blob, filename);  
  },
  tar_savefile:function(filename){
    let buffer=UI.numworks_buffer;
    let finfo=UI.tar_fileinfo(buffer,0);
    let s=finfo.length;
    if (s==0) return 0;
    for (let i=0;i<s;++i){
      let cur=finfo[i];
      if (cur.name!=filename) continue;
      let view=new Uint8Array(cur.size);
      let offset=cur.header_offset+512;
      let buf=new Uint8Array(buffer);
      for (let j=0;j<cur.size;++j)
	view[j]=buf[offset+j];
      //console.log(cur,offset,view);
      let blob = new Blob([view], {type: "octet/stream"});
      saveAs(blob, filename);  
      return;
    }
  },
  numworks_gettar:function(calc){
    if (calc==0){
      UI.numworks_gettar_(calc);
      return;
    }
    UI.calc=2;
    UI.nws_connect();
    window.setTimeout(UI.numworks_gettar_,100,calc);
  },
  numworks_gettar_:async function(calc){
    console.log(calc,UI.calculator);
    if (calc && (UI.calculator==0 || !UI.calculator_connected)){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return;
    }
    if (UI.numworks_buffer!=0){
      if (!confirm('Attention, la personnalisation courante va etre effacee! Continuer?'))
	return;
    }
    UI.tar_first_modif_offset=0;
    if (calc==0)
      UI.numworks_buffer=await UI.loadfile('apps.tar');
    else
      UI.numworks_buffer=await UI.calculator.get_apps();
    let finfo=UI.tar_fileinfo(UI.numworks_buffer,1),s=finfo.length;
    if (calc && s>0)
      UI.tar_first_modif_offset=finfo[s-1].header_offset+UI.tar_filesize(finfo[s-1].size);
    // window.setTimeout(UI.tar_fileinfo,1000,UI.numworks_buffer);
    //UI.print("Archive flash:\n"+UI.numworks_tarinfo(UI.numworks_buffer));
  },
  numworks_sendtar:function(){
    UI.calc=2;
    UI.nws_connect();
    window.setTimeout(UI.numworks_sendtar_,100);
  },
  numworks_sendtar_:async function(){
    if (!UI.numworks_chk_buffer())
      return;
    if (!UI.calculator || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return;
    }
    $id('perso').style.display='none';    
    if (UI.tar_first_modif_offset){
      let finfo=UI.tar_fileinfo(UI.numworks_buffer,0);
      let s=finfo.length;
      if (s==0) return;
      let start=65536*Math.floor(UI.tar_first_modif_offset/65536);
      let size=finfo[s-1].header_offset+UI.tar_filesize(finfo[s-1].size)-start;
      let buf=new Uint8Array(size);
      let src=new Uint8Array(UI.numworks_buffer);
      for (let i=0;i<size;++i)
	buf[i]=src[start+i];
      let buffer=buf.buffer;
      console.log('sendtar',UI.tar_first_modif_offset,start,size,buffer.byteLength);
      UI.calculator.device.startAddress = 0x90200000+start;
      UI.calculator.device.logProgress(0,'apps');
      res=await UI.calculator.device.do_download(UI.calculator.transferSize, buffer, false);      
    }
    else {
      UI.calculator.device.startAddress = 0x90200000;
      UI.calculator.device.logProgress(0,'apps');
      res=await UI.calculator.device.do_download(UI.calculator.transferSize, UI.numworks_buffer, false);
    }
    $id('perso').style.display='inline';    
    UI.print('apps OK, press RESET on calculator back');
  },
  numworks_certify:function(sigfile,rwcheck=false){
    $id('persoinit').style.display='none';    
    UI.calc=2;
    UI.nws_connect();
    window.setTimeout(UI.numworks_certify_,100,sigfile,rwcheck);
  },
  numworks_certify_:async function(sigfile,rwcheck){
    if (UI.calculator==0 || !UI.calculator_connected){
      alert(UI.langue==-1?UI.chkmsgfr:chkmsgen);
      if (UI.calculator) UI.calculator.stopAutoConnect();
      return -1;
    }
    if (rwcheck)
      if (!confirm(UI.langue==-1?"Ce test necessite l'accord du proprietaire de la calculatrice, il dure environ 1 minute. Poursuivre?":'This test requires the calculator owner authorization. It will take about 1 minute. Perform?'))
	return -1;
    //else alert(UI.langue==-1?'Le test va prendre environ 20 secondes':'Test will take about 20 seconds');
    UI.print('========');
    let internal=await UI.calculator.get_internal_flash();
    //console.log(sigfile);
    let res=await UI.sig_check(sigfile,internal,'delta.internal.bin');
    if (!res){
      alert(UI.langue==-1?'Flash interne non certifiee. Cliquer sur installer KhiCAS pour installer un firmware certifie.':'Internal flash not certified');
      return 1;
    }
    UI.print('Internal flash OK');
    let external=await UI.calculator.get_external_flash();
    res=await UI.sig_check(sigfile,external,'delta.external.bin');
    if (!res){
      alert(UI.langue==-1?'Flash externe non certifiee. Cliquer sur installer KhiCAS pour installer un firmware certifie.':'External flash not certified');
      return 2;
    }
    UI.print('External flash OK');
    let apps=await UI.calculator.get_apps();
    res=await UI.sig_check(sigfile,apps,'apps.tar');
    if (!res){
      alert(UI.langue==-1?'Applications non certifiees. Cliquer sur installer KhiCAS pour installer un firmware certifie.':'Applications not certified');
      return 3;
    }
    UI.print('Apps OK');
    if (rwcheck){
      UI.print('R/W check 0x91200000');
      res=await UI.calculator.rw_check(0x90120000,0xe0000);
      if (!res){
	alert(UI.langue==-1?'Echec du test lecture/ecriture':'Read/Write test failure');
	return 4;
      }
      UI.print('R/W check 0x90740000');
      res=await UI.calculator.rw_check(0x90740000,0xa0000);
      if (!res){
	alert(UI.langue==-1?'Echec du test lecture/ecriture':'Read/Write test failure');
	return 4;
      }
      UI.print('R/W OK');
    }
    alert(UI.langue==-1?'Firmware certifie':'Firmware certified');
    return 0;
  },
  htmlbuffer:'',
  htmlcheck:true,
  print:function(text){
    let element = $id('output');
    //console.log(text.charCodeAt(0));
    if (text.length == 1 && text.charCodeAt(0) == 12) {
      element.innerHTML = '';
      return;
    }
    if (text.length >= 1 && text.charCodeAt(0) == 2) {
      console.log('STX');
      UI.htmlcheck = false;
      UI.htmlbuffer = '';
      return;
    }
    if (text.length >= 1 && text.charCodeAt(0) == 3) {
      console.log('ETX');
      UI.htmlcheck = true;
      element.style.display = 'inherit';
      element.innerHTML += UI.htmlbuffer;
      UI.htmlbuffer = '';
      element.scrollTop = 99999;
      return;
    }
    if (UI.htmlcheck) {
      // These replacements are necessary if you render to raw HTML
      text = '' + text;
      console.log(text);
      text = text.replace(/&/g, "&amp;");
      text = text.replace(/</g, "&lt;");
      text = text.replace(/>/g, "&gt;");
      text = text.replace(/\n/g, '<br>');
      text += '<br>';
      element.style.display = 'inherit';
      element.innerHTML += text; // element.value += text + "\n";
      element.scrollTop = 99999; // focus on bottom
    } else UI.htmlbuffer += text;
    element.scrollIntoView();
  },
};

