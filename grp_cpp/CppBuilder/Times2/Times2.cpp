//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#pragma hdrstop

#include "Times2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

//---------------------------------------------------------------------------
void TForm1::DoMult(void)
{
char buf[100];
double value;

Edit1->GetTextBuf(buf,100);
sscanf(buf,"%lf",&value);
sprintf(buf,"%lg",2.0 * value);
Edit2->SetTextBuf(buf);
}


//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
DoMult();  // Make sure initial values are correct
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Edit1KeyPress(TObject *Sender, char &Key)
{
if(Key == '\r')DoMult();  // Update if user presses ENTER
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Edit1Exit(TObject *Sender)
{
DoMult();  // Update on loss of focus        
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
exit(0);  // End execution        
}
//---------------------------------------------------------------------------
