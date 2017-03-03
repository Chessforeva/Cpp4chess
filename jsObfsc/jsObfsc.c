
// Tiny C support for LinkRes2Exe
#ifdef __TINYC__
  int dummy __attribute__ ((section(".rsrc")));
#endif
#define Icon_Resourced 30500
// if compile without resource then set LoadIcon as it is in comments


#include <windows.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

char *AboutTxt = "This is a free source win32 tool that reduces\n"
	"javascript code to be smaller for browser,\n"
	" non-readable for developer anymore.\n\n"
	"Written for TinyC small C compiler,\n"
	" much thanks to http://www.tinycc.org\n\n"
	"Steps 1-3. minifier (removes redundancies)\n"
	"Steps 4-6. obfuscator (shortens names),\n\n"
	"by Chessforeva, 2016\n"
	"http://github.com/Chessforeva/Cpp4chess\n";

// added some definitions for this project, modified gcc headers for tcc
#include <commctrl.h>
#include <commdlg.h>

// Separated modules sources
#include "minfy.c"
#include "obfsc.c"

// window sizes
#define WINW 800
#define WINH 600
#define WINW2 WINW-40
#define WINW3 360

// ids
enum
{
 IDT_TIMER1,
 IDR_MYMENU,
 ID_OPEN,
 ID_SAVE,
 ID_QUIT,
 ID_MINFY,
 ID_PREOKW,
 ID_OBFSC,
 ID_SRCEDIT,
 ID_MRESEDIT,
 ID_KWDGRID,
 ID_ORESEDIT,
 ID_ABOUT
};

//======== Buffers

UINT BSize;											// buffer size
CHAR *SrcPtr, *DstPtr, *Ptr, *KwlPtr;			// pointers to buffers
char buf[1024];
int i,pi;						// some loop variables
BOOL kwd2rd;

//======== useful globals
HINSTANCE hinst;    /* This hinstance */
HMENU hMenu;        /* Menu */
HWND hwnd;          /* This is the handle for our window */
RECT hwrect;        /* Rect of the main window */
UINT FontSize;		  /* default font size */
HWND hwTab;         /* Tab window handle */
HWND hwSrcEdit;     /* Src.Editbox window handle */
HWND hwT0,hwT1,hwT2,hwT3,hwTa,hwT4,hwTb,hwT5;    /* Textboxes */
HWND hwTp0,hwTp1,hwTp2;   /* Textboxes for carret position line text */
HWND hwM0;          /* CkBx min.option to remove ...redundant tabs and spaces */
HWND hwM1;          /* CkBx ...linebreaks */
HWND hwM2;          /* CkBx ...comments */
HWND hwM3;          /* CkBx ...smart remove */
HWND BtMinfy;       /* Button Minify */
HWND hwMResEdit;     /* Editbox window handle for result after minify */
HWND hwO0;          /* CkBx obf.option  functions*/
HWND hwO1;          /* CkBx ...variables*/
HWND hwO2;          /* CkBx replace funcs in html also*/
HWND hwO3;          /* CkBx all keywords */
HWND hwOLen;		/* Input field for minimal length of keywords */
HWND BtPrepKw;		/* Button Prepare keywords */
HWND hwKwGrid;		/* Keyword Grid window handle */
HWND BtObfsc;		/* Button Obfuscate */
int kwdlen;			/* keyword len variable */
HWND hwOResEdit;	/* Editbox window handle for result after obfuscate */

//======== MENU

HMENU addPopup( CHAR* String )
{
 HMENU Popup = CreatePopupMenu();
 AppendMenu(hMenu,  MF_STRING | MF_POPUP, (UINT)Popup, String );
 return Popup;
}

void addPopItem( HMENU Popup, UINT Id, CHAR* String )
{
 AppendMenu(Popup, MF_STRING, Id, String);
}


void CreateMainMenu()
{
    hMenu = CreateMenu();

    HMENU P0 = addPopup("File");
		addPopItem(P0, ID_OPEN, "Open");
		addPopItem(P0, ID_SAVE, "Save");
		addPopItem(P0, ID_QUIT, "Quit");

    HMENU P1 = addPopup("Action");
		addPopItem(P1, ID_MINFY, "Minify");
		addPopItem(P1, ID_OBFSC, "Obfuscate");
		
    HMENU P2 = addPopup("Help");
		addPopItem(P2, ID_ABOUT, "About");

    SetMenu(hwnd, hMenu);
}


/* ------------------
 win32 based user controls source for TCC compiler
*/

/* sets text value to a field */
void setText(HWND H, CHAR *String) { SendMessage(H, WM_SETTEXT, 0, (LPARAM)String); }

/* Creates a label text field, sets text value */
HWND addLabel( CHAR *String, UINT x, UINT y, UINT w, UINT h )
{
 HWND H = CreateWindow(
    "STATIC", /* predefined class */
    NULL, /* no window title */
    WS_CHILD | WS_VISIBLE | ES_LEFT,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    NULL, /* edit control ID */
    hinst,
    NULL); /* pointer not needed */

 setText(H, String);
 return H;
}

/* Creates a multiline editable text field */
HWND addMLEdit( UINT id, UINT x, UINT y, UINT w, UINT h )
{
 return CreateWindow(
    "EDIT", /* predefined class */
    NULL, /* no window title */
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_THICKFRAME | WS_TILED | WS_BORDER |
    ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    (HMENU) id, /* edit control ID */
    hinst,
    NULL); /* pointer not needed */
}

/* Creates a simple editable field */
HWND addInputField( CHAR *String, UINT x, UINT y, UINT w, UINT h )
{
 HWND H = CreateWindow(
    "COMBOBOX", /* predefined class */
    NULL, /* no window title */
    WS_CHILD | WS_VISIBLE | ES_LEFT,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    NULL, /* control ID */
    hinst,
    NULL); /* pointer not needed */

 setText(H, String);
 return H;
}

// gets field value
void getInputField(HWND H, CHAR *str, UINT len)
{
 SendMessage(H,WM_GETTEXT,len,str);
}

/* Creates a checkbox field */
HWND addCheckBox( CHAR *String, UINT x, UINT y, UINT w, UINT h, BOOL checked )
{
 HWND H = CreateWindow(
    "BUTTON", /* predefined class */
    NULL, /* no window title */
    WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE | ES_LEFT,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    NULL, /* control ID */
    hinst,
    NULL); /* pointer not needed */

 setText(H, String);
 if(checked) PostMessage(H, BM_SETCHECK,BST_CHECKED,0);
 return H;
}


// gets checkbox status
BOOL isCheckBoxChecked(HWND H)
{
 return (SendMessage(H,BM_GETCHECK,0,0)==BST_CHECKED);
}

/* Creates a button field */
HWND addButton( UINT Id, CHAR *String, UINT x, UINT y, UINT w, UINT h )
{
 HWND H = CreateWindow(
    "BUTTON", /* predefined class */
    NULL, /* no window title */
    WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | ES_LEFT,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    Id, /* control ID */
    hinst,
    NULL); /* pointer not needed */

 setText(H, String);
 return H;
}

/* A GRID */

/* Creates a simple grid */
HWND addGrid( UINT Id, UINT x, UINT y, UINT w, UINT h )
{
 return CreateWindow(
    WC_LISTVIEW, /* predefined class */
    NULL, /* no window title */
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_EDITLABELS,
    x, y, w, h, /* set size in WM_SIZE message */
    hwnd, /* parent window */
    Id, /* control ID */
    hinst,
    NULL); /* pointer not needed */
}

LVCOLUMN lvc;
LVITEM lvi = {0};

int CreateColumn(HWND H, int iCol, char *Text, int width)
{
 lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
 lvc.fmt = LVCFMT_LEFT;
 lvc.cx = width;
 lvc.pszText = Text;  
 lvc.iSubItem = iCol;
 return ListView_InsertColumn(H, iCol, &lvc);
}

int rowAdded; // this saves last added row

void AddRow(HWND H)			// adds a row, returns row index
{
 lvi.mask = LVIF_TEXT;
 lvi.pszText = "";
 lvi.iItem = 999999;
 rowAdded = ListView_InsertItem(H,&lvi);
}

void SetColValue(HWND H, int col, char *Text)
{
 lvi.mask = LVIF_TEXT;
 lvi.pszText = Text;
 ListView_SetItemText(H, rowAdded, col, Text);
}

void ClearListbox(HWND H)
{
 ListView_DeleteAllItems(H);
}


/* ------------------ */

//======== Tab section

void addTab( UINT seq, CHAR *Name )
{
TC_ITEM T;
T.mask=TCIF_TEXT;
T.pszText=Name;
TabCtrl_InsertItem(hwTab,seq,&T);
}

void CreateTabs()
{
hwTab = CreateWindow( WC_TABCONTROL, "",
    WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
    0, 0, hwrect.right, hwrect.bottom,
    hwnd, NULL, hinst, NULL
    );

addTab(0,"Step 1");
addTab(1,"Step 2");
addTab(2,"Step 3");
addTab(3,"Step 4");
addTab(4,"Step 5");
addTab(5,"Step 6");
}

//======== Source editbox

void CreaTexts()
{

 //Step1
 hwT0 = addLabel("Copy/paste script source here,  or open a js/html file", 10,50,WINW3,30);
 hwSrcEdit = addMLEdit(ID_SRCEDIT,10, 80, WINW2, 450);
 hwTp0 = addLabel("", 400,50,100,30);

 //Step2
 hwT1 = addLabel("Set options for javascript minifier:", 10,50,WINW3,30);
 hwM0 = addCheckBox("remove redundant spaces and tabs", 10,90,WINW3,30, TRUE);
 hwM1 = addCheckBox("reduce linebreaks", 10,120,WINW3,30, TRUE);
 hwM2 = addCheckBox("remove comments", 10,150,WINW3,30, TRUE);
 hwM3 = addCheckBox("remove (;)s", 10,180,WINW3,30, TRUE);
 BtMinfy  = addButton(ID_MINFY, "Minify", 10,220,100,30);

 //Step3
 hwT2 = addLabel("Minified result:", 10,50,WINW3,30);
 hwMResEdit = addMLEdit(ID_MRESEDIT,10, 80, WINW2, 450);
 hwTp1 = addLabel("", 400,50,100,30);

 //Step4
 hwT3 = addLabel("Set options for javascript obfuscator:", 10,50,WINW3,30);
 hwO0 = addCheckBox("functions", 10,90,WINW3,30, TRUE);
 hwO1 = addCheckBox("variables", 10,120,WINW3,30, TRUE);
 hwTa = addLabel("minimal length of keywords", 56,164,WINW3,30);
 hwO2 = addCheckBox("also in html", 10,200,WINW3,30, FALSE);
 hwO3 = addCheckBox("all keywords", 10,230,WINW3,30, FALSE);
 hwOLen = addInputField("3", 10,160,40, FontSize+8  );
 BtPrepKw  = addButton(ID_PREOKW, "Prepare keywords", 10,270,180,30);
 
 //Step5
 hwT4 = addLabel("Clear/edit the \"new\" generated name, then obfuscate.", 10,50,WINW3,30);
 hwTb = addLabel("", 400,50,WINW3,100);
 hwKwGrid = addGrid(ID_KWDGRID,10, 100, WINW2, 450);
 CreateColumn(hwKwGrid,0,"new", 50);
 CreateColumn(hwKwGrid,1,"keyword", 220);
 CreateColumn(hwKwGrid,2,"cnt", 60);
 CreateColumn(hwKwGrid,3,"description", 430);
 BtObfsc  = addButton(ID_OBFSC, "Obfuscate", 600,50,140,30);
 
 //Step6
 hwT5 = addLabel("Obfuscated result:", 10,50,WINW3,30);
 hwOResEdit = addMLEdit(ID_ORESEDIT,10, 80, WINW2, 450);
 hwTp2 = addLabel("", 400,50,100,30);
  
 PostMessage(hwnd, WM_NOTIFY, 0, 0);        // display and hide
}

// calls for steps 1-3
void requireMinified()
{
 if((LRESULT)SendMessage(hwMResEdit,WM_GETTEXTLENGTH,0,0)==0)
  SendMessage(hwnd, WM_COMMAND, ID_MINFY, 0);
}

// calls for steps 4-5
void requirePrepKW()
{
 pi=-1;
 if((pi = ListView_GetNextItem(hwKwGrid, pi, LVNI_ALL ))<0)
  SendMessage(hwnd, WM_COMMAND, ID_PREOKW, 0);
}



//----- LineNo displaying for editboxes
//
UINT pre_pos;
void ClearDispNo() { pre_pos = -1; }


// displays current line number for editor
void DispLineNo()
{
 UINT step = TabCtrl_GetCurSel(hwTab);
 HWND H = NULL,Hp;
 
 if(step==0) { H=hwSrcEdit; Hp=hwTp0; }
 if(step==2) { H=hwMResEdit; Hp=hwTp1; }
 if(step==5) { H=hwOResEdit; Hp=hwTp2; }
  
 if(H!=NULL)
  {

  //LRESULT len = (WPARAM)SendMessage(H,WM_GETTEXTLENGTH,0,0);
  LRESULT pos = (WPARAM)SendMessage(H,EM_LINEINDEX,-1,0);
  if(pre_pos!=pos)
   {
   pre_pos = pos;
   Ptr = malloc(pos+2);
   SendMessage(H,WM_GETTEXT,pos+1,Ptr);
   Ptr[pos]=0;
   char c, *p=Ptr;
   UINT row=1;
   for(c=*p;c!=0;c=*(++p))
    {
     if(c==13 && *(p+1)==10) p++;
     if(c==13||c==10) row++;    // skip LF,CR
    }
   sprintf(buf,"Line: %d", row);
   setText(Hp,buf);
   free(Ptr);
   }
  }
}


//======== Files

// for file operations
OPENFILENAME ofn;       // common dialog box structure
char szFile[2048];      // buffer for file name
HANDLE hf;              // file handle
size_t sz;              // filesize
LPDWORD Fsz;
OVERLAPPED ol = {0};

CHAR* FileWld = "Javascript\0*.js\0Html\0*.htm*\0All\0*.*\0";
CHAR* FileKwd = "Keyword\0*.kwd\0";


CHAR Ext[9] = {0};			// extension

void GetExtns()
{
	CHAR *p=szFile,c;
	for(c=*p; c!=0; c=*(++p))
	 if(c=='.') memcpy(Ext,p,9);
}

//======== File Open functions

void UserOpenFile()
{
UINT tab = TabCtrl_GetCurSel(hwTab);

// Initialize OPENFILENAME
ZeroMemory(&ofn, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.hwndOwner = hwnd;
ofn.lpstrFile = szFile;
ofn.lpstrFile[0] = '\0';
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = ( tab == 4 ? FileKwd : FileWld );
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

// Display the Open dialog box.

if (GetOpenFileName(&ofn)==TRUE)
 {
 GetExtns();		// obtain extension
 hf = CreateFile(ofn.lpstrFile,
                    GENERIC_READ,
                    FILE_SHARE_READ,       // share for reading
                    (LPSECURITY_ATTRIBUTES) NULL,
                    OPEN_EXISTING,         // existing file only
                    FILE_ATTRIBUTE_NORMAL,
                    (HANDLE) NULL);	// address of structure with initialization data
 if(hf==INVALID_HANDLE_VALUE)
  {
     MessageBox(hwnd, "Error! Can not open the file!", "", MB_OK );
  }
 else
  {
    sz = GetFileSize(hf, &Fsz);     // get file size
    if(sz!=INVALID_FILE_SIZE)
    {
    SrcPtr = malloc(sz+1);      // allocate memory
    SrcPtr[sz]=0;

    if( ReadFileEx(hf, SrcPtr, sz, &ol, NULL) )     // read file in buffer
        {
	if(tab == 4) kwd2rd = TRUE;
	else
		{
		setText( hwSrcEdit, SrcPtr );   // Set text in Edit control
			ClearListbox(hwKwGrid);
			setText(hwMResEdit,"");		// clear previous
			setText(hwOResEdit,"");
		}
        }
    }
    CloseHandle(hf);
    if(tab != 4) free(SrcPtr);       // release memory block
  }
 }

}

// selects tab to save
int selSaveTab()
{
 UINT rTab=2;
 UINT step = TabCtrl_GetCurSel(hwTab);
 if(step==2||step==4||step==5) rTab = step;
 else
  if((WPARAM)SendMessage(hwOResEdit,WM_GETTEXTLENGTH,0,0)>0) rTab=5;
 return rTab;
}

//======== File Save functions

void UserSaveFile()
{
HWND H;

UINT tab = TabCtrl_GetCurSel(hwTab);
if(tab!=4)
 {
 H = ( tab == 5 ? hwOResEdit : hwMResEdit );
 (LRESULT)BSize = (WPARAM)SendMessage(H,WM_GETTEXTLENGTH,0,0);
 }
 
if(BSize>0)
{

// Initialize
ZeroMemory(&ofn, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.hwndOwner = hwnd;
ofn.lpstrFile = szFile;
ofn.lpstrFile[0] = '\0';
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = ( tab == 4 ? FileKwd : FileWld );
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = TEXT("Save File As");
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_HIDEREADONLY;

// Display the Save dialog box.

if (GetSaveFileName(&ofn)==TRUE)
 {
 	if( tab == 4 ) strcpy(Ext, ".kwd");
	else if(strlen(Ext)==0) strcat(Ext, ".js");

 	if( strstr(szFile,Ext)==NULL ) strcat(szFile,Ext);

 	
  // verify existance
  hf = CreateFile(ofn.lpstrFile,
                    GENERIC_READ,
                    FILE_SHARE_READ,       // share for reading
                    (LPSECURITY_ATTRIBUTES) NULL,
                    OPEN_EXISTING,         // existing file only
                    FILE_ATTRIBUTE_NORMAL,
                    (HANDLE) NULL);	// address of structure with initialization data
  if(hf!=INVALID_HANDLE_VALUE)
  {
  	CloseHandle(hf);             
    if( MessageBox(hwnd, "File exists. Overwrite it?", ofn.lpstrFile, MB_YESNO )!=IDYES ) return;
  }
   
 hf = CreateFile(ofn.lpstrFile,
                    GENERIC_WRITE,
                    0,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    CREATE_ALWAYS,         // existing file only
                    FILE_ATTRIBUTE_NORMAL,
                    (HANDLE) NULL);	// address of structure with initialization data
 if(hf==INVALID_HANDLE_VALUE)
  {
     MessageBox(hwnd, "Error! Can not write to file!", "", MB_OK );
  }
 else
  {
  	if(tab!=4)
		{
		SrcPtr = malloc(BSize+1);
		SendMessage(H,WM_GETTEXT,BSize+1,SrcPtr);
		BSize = strlen(SrcPtr);	// recount
		}

    if( WriteFileEx(hf, SrcPtr,  BSize, &ol, NULL) ){};     // write buffer to file
    CloseHandle(hf);
    free(SrcPtr);       // release memory block
  }
 }

}

}


/* When user resizes application window */

// Adjust sizes for windows
// x,y is left-top-corner position

void AdjSize( HWND H, UINT x, UINT y )
{
RECT hwR, hr;
UINT Hnew,Wnew,Hold,Wold,DW,DH,width, height;
GetWindowRect(H, &hr);
GetClientRect(hwnd, &hwR);
width = hr.right - hr.left;
height = hr.bottom - hr.top;

Wnew = hwR.right - hwR.left;
Hnew = hwR.bottom - hwR.top;

Wold = hwrect.right - hwrect.left;
Hold = hwrect.bottom - hwrect.top;

DH=Hnew-Hold; DW=Wnew-Wold;     // calculates difference

if(DH|DW) MoveWindow( H, x, y, width+DW, height+DH, TRUE );
ClearDispNo();
}

#define ShowIf(x)  (step==x ? SW_RESTORE : SW_HIDE)
#define SHWHD(h,x) ShowWindow(h, ShowIf(x));

void ShowHideAll( UINT step )
{
 UINT curTab = TabCtrl_GetCurSel(hwTab);
 if(kwd2rd)
  {
  step = curTab;
  kwd2rd = FALSE;
  }
 if(curTab!=step) TabCtrl_SetCurSel(hwTab,step);

    // Step1
 SHWHD(hwSrcEdit,0); SHWHD(hwT0,0); SHWHD(hwTp0,0);
    // Step2
 SHWHD(hwT1,1); SHWHD(hwM0,1); SHWHD(hwM1,1); SHWHD(hwM2,1); SHWHD(hwM3,1); SHWHD(BtMinfy,1);
    // Step3
 SHWHD(hwMResEdit,2); SHWHD(hwT2,2); SHWHD(hwTp1,2);
    // Step4
 SHWHD(hwT3,3); SHWHD(hwO0,3); SHWHD(hwO1,3); SHWHD(hwTa,3); SHWHD(hwOLen,3); SHWHD(hwO2,3); SHWHD(hwO3,3); SHWHD(BtPrepKw,3);
    // Step5
 SHWHD(hwT4,4); SHWHD(hwKwGrid,4); SHWHD(BtObfsc,4);  SHWHD(hwTb,4);
    // Step6
 SHWHD(hwOResEdit,5); SHWHD(hwT5,5); SHWHD(hwTp2,5);
 
 ClearDispNo();
}


/* Calls Minify command */
void Call_Minify()
{
 (LRESULT)BSize = (WPARAM)SendMessage(hwSrcEdit,WM_GETTEXTLENGTH,0,0);
 BSize++;
 SrcPtr = malloc(BSize+2);
 SendMessage(hwSrcEdit,WM_GETTEXT,BSize,SrcPtr+1);
 SrcPtr[0]=0;
 DstPtr = malloc(BSize+2);
 DstPtr[0]=0;
 
 Minfy(DstPtr, SrcPtr+1, isCheckBoxChecked(hwM0), isCheckBoxChecked(hwM1), isCheckBoxChecked(hwM2), isCheckBoxChecked(hwM3));
 
 setText(hwMResEdit,DstPtr);
 free(SrcPtr);
 free(DstPtr);
 ClearDispNo();
 ClearListbox(hwKwGrid);
 setText(hwOResEdit,"");
}

CHAR *rlist_p;		// pointer to kw.list
// reads a data string from buffer
CHAR *rlist_get()
{
	CHAR *p=rlist_p, c=*p;
	for(;c!=0 && c!=1; c=*(++rlist_p));
	if(c==1) *(rlist_p++)=0;
	return p; 
}

/* Calls Prepare keywords command */
void Call_PrepKeywords()
{
 char kwlenbf[5];		// length of keywords
 CHAR *p;
 (LRESULT)BSize = (WPARAM)SendMessage(hwMResEdit,WM_GETTEXTLENGTH,0,0);
 BSize++;
 SrcPtr = malloc(BSize+2);
 SendMessage(hwMResEdit,WM_GETTEXT,BSize,SrcPtr+1);
 SrcPtr[0]=0;
 getInputField(hwOLen, &kwlenbf, 4);
 KwlPtr = malloc(524288);			//  512Kb buffer
 KwlPtr[0]=0;
 kwdlen = atoi(kwlenbf);
 if(kwdlen<1) kwdlen=1;
 sprintf(kwlenbf,"%d",kwdlen);
 setText(hwOLen,kwlenbf);
 
 PrepKeywords( SrcPtr+1, KwlPtr, isCheckBoxChecked(hwO0), isCheckBoxChecked(hwO1), isCheckBoxChecked(hwO3), kwdlen, hwTb );
 
 setText(hwTb,"");
 ClearListbox(hwKwGrid);
 
 for(rlist_p = KwlPtr; *(rlist_p)!=0; )
 {
	AddRow(hwKwGrid);
	for(i=0;i<4;i++)
		{
		p = rlist_get();
		if(i==0) { for(;*p==' ';p++); }	// skip spaces
		if(i==3)
		 {
		 buf[0]=0;
		 if(*p!=0)
			{
			sprintf(buf, " %s%s%s%s%s", (*(p++)=='I' ? "html, ":""), (*(p++)=='F' ? "function, ":""), (*(p++)=='V' ? "variable, ":""),
			(*(p++)=='O' ? "object, " :""), (*(p++)=='P' ? "obj.property, ":"") );
			buf[strlen(buf)-2]=0;
			}
		 p=buf;
		 }
		SetColValue(hwKwGrid,i,p);
		}
 }
 
 free(KwlPtr);
 free(SrcPtr);
}

// read listbox items one by one
void GetListBoxItem( CHAR *indst )
{
 lvi.iItem = pi;
 lvi.iSubItem = i++;
 lvi.cchTextMax=1000;			// doesn't work otherwise :)
 lvi.mask=LVIF_TEXT;
 if(ListView_GetItem(hwKwGrid, &lvi)) strcpy(indst,lvi.pszText); else indst[0]=0; 
 lvi.cchTextMax = 0;
 lvi.mask = 0;
 lvi.iSubItem = 0;
}
 
/* Obfuscates as in grid  table */
void Call_Obfuscate()
{
 CHAR kwrd[100],nwkw[100];
 (LRESULT)BSize = (WPARAM)SendMessage(hwMResEdit,WM_GETTEXTLENGTH,0,0);
 BSize++;
 SrcPtr = malloc(BSize*2);			// +some reserve
 SendMessage(hwMResEdit,WM_GETTEXT,BSize,SrcPtr+1);
 SrcPtr[0]=0;
 
 for(pi=-1;;)
	{
	if((pi = ListView_GetNextItem(hwKwGrid, pi, LVNI_ALL ))<0) break;
	i=0;
	GetListBoxItem(nwkw);	// new keyword
	GetListBoxItem(kwrd);	// keyword
	GetListBoxItem(buf);	// cnt,ignore
	GetListBoxItem(buf);	// descr.
	if( nwkw[0]!=0 )
		{
		Obfuscate( SrcPtr+1, nwkw, kwrd, isCheckBoxChecked(hwO2), (strstr("func",buf)!=NULL) );
		}
	}
 setText(hwOResEdit,SrcPtr+1);
 free(SrcPtr);
}

// --------
// User can save table of keywords
//  (added 02.2017)
// ------
void toSaveKwrds()
{
 UINT tab = TabCtrl_GetCurSel(hwTab);
 UINT n;
 CHAR *p;
 if( tab==4 )
 {
 SrcPtr = malloc(524288);			//  512Kb buffer
 p = SrcPtr; *p = 0;
 for(pi=-1;;)
	{
	if((pi = ListView_GetNextItem(hwKwGrid, pi, LVNI_ALL ))<0) break;
	i=0;
	for(n = 4; n>0; n-- )	// new keyword,keyword,cnt,descr.
		{
		GetListBoxItem(buf);
		strcpy(p,buf); p+=strlen(buf);
		*(p++)=13;
		}
	}
 *(p++)=0;
 BSize = strlen(SrcPtr);
 }
}

// and read from file keywords prepared before
void toReadKwrds()
{
 UINT tab = TabCtrl_GetCurSel(hwTab);
 UINT n, L=0;
 CHAR *p, *q, c;
 if( tab==4 && kwd2rd )
 {
  ClearListbox(hwKwGrid);
  p = SrcPtr;
  while(*p!=0 && L<sz)
	{
	AddRow(hwKwGrid);
	for(i=0;*p!=0 && i<4;i++)
		{
		q = buf;
		for(c=1; c; q++,p++,L++)
			{
			c = *p;
			if(c==13) c = 0; 
			*q = c;
			}	
		SetColValue(hwKwGrid,i,buf);
		}
	}
  free(SrcPtr);	
 }
}

#define GetLbStr(x,v) { ListView_GetItemText(hwKwGrid, x, lParamSort, buf, 999 ); \
						v = (lParamSort==2 ? atoi(buf) : 100000*buf[0]+100*buf[1]+buf[2] ); }

int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
 long v1,v2,v;
 GetLbStr(lParam1,v1); GetLbStr(lParam2,v2);
 return (int)(v1-v2);
}

// grid handler
BOOL GridNotify(WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO *pI = (LV_DISPINFO *) lParam, I = *pI;
	NM_LISTVIEW *m = (NM_LISTVIEW *)lParam;

	
	switch (I.hdr.code) {
		case NM_CLICK:
			break;
		case NM_DBLCLK:
			ListView_SetItemText(hwKwGrid, I.item, 0, "");
			return TRUE;
			break;
		case LVN_BEGINLABELEDIT:
			break;
		case LVN_ENDLABELEDIT:
			if(I.item.pszText != NULL) { return TRUE; }
			break;
		case LVN_COLUMNCLICK:
			// The user clicked on one of the column headings - sort by
			// this column.
			ListView_SortItemsEx( m->hdr.hwndFrom,
								ListViewCompareProc,
								(LPARAM)(m->iSubItem));
			break;

		break;
       }
	return FALSE;
}


//======== WINDOW


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "jsObfuscWin32app";

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    
		// or without resource LoadIcon(NULL,IDI_APPLICATION)
    wincl.hIcon = LoadIcon(hThisInstance, Icon_Resourced);
    wincl.hIconSm = LoadIcon(hThisInstance, Icon_Resourced);
    
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);                 /* menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;
        
	InitCommonControls();
		
    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "jsObfsc v1.2 - a simple javascript code minifier+obfuscator",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           WINW,                 /* The programs width */
           WINH,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);
    // Create the main menu
	CreateMainMenu();

    hinst = hThisInstance;

	DrawText(GetDC(hwnd), " ", 1, &hwrect, DT_CALCRECT);		// obtain text size
	FontSize = hwrect.bottom-hwrect.top;										// 16

    GetClientRect(hwnd, &hwrect);

	CreateTabs();
	CreaTexts();
				

    // Set a timer. 
    SetTimer(hwnd,             // handle to main window 
        IDT_TIMER1,            // timer identifier 
        200,                  //  1/5 second interval 
        (TIMERPROC) NULL);     // no timer callback 
        
    ClearDispNo();
    
    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)                  /* handle the messages */
    {

		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case ID_OPEN:
					kwd2rd = FALSE;
					UserOpenFile();
					toReadKwrds();					
 					ShowHideAll( 0);
					break;
				case ID_SAVE:
					ShowHideAll( selSaveTab() );
					toSaveKwrds();
					UserSaveFile();
					break;
				case ID_QUIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case ID_MINFY:
					ShowHideAll(2);
					Call_Minify();
					break;
				case ID_PREOKW:
					requireMinified();
					ShowHideAll(4);
					Call_PrepKeywords();
					break;
				case ID_OBFSC:
					requirePrepKW();
					ShowHideAll(5);
					Call_Obfuscate();
					break;
				case ID_ABOUT:
					MessageBox(hwnd, AboutTxt, "About", MB_OK);
					break;
			}
			break;

		case WM_TIMER: 
 
			switch (wParam) 
			{ 
			case IDT_TIMER1: 
			// process 1/5 second timer 
			DispLineNo();
			break;
			}
            
 		case WM_NOTIFY:
 		
			if( wParam == ID_KWDGRID) return GridNotify( wParam, lParam );
           
			switch (HIWORD(wParam))
			{
			case 0:
			// menu command processing
			case TCN_SELCHANGE:
				{
			/* Display fields according to tab selected */
			ShowHideAll( TabCtrl_GetCurSel(hwTab) );
			}
			break;
        		
			}

		case WM_SIZE:

			AdjSize(hwTab,0,0);
			AdjSize(hwSrcEdit,10,80);
			AdjSize(hwMResEdit,10,80);
			AdjSize(hwKwGrid,10,100);
			AdjSize(hwOResEdit,10,80);
			GetClientRect(hwnd, &hwrect);
			break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


