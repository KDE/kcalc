/*
    $Id$

    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef QTCALC_H
#define QTCALC_H

#include <queue>
using std::queue;

class QLabel;
class QListBox;
class QTimer;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QWidget;
class QPopupMenu;
class KHelpMenu;
class ConfigureDialog;
class DLabel;
class KPushButton;

#include <kdialog.h>
#include "stats.h"


#ifdef HAVE_CONFIG_H
	#include "../config.h"
#endif

// IMPORTANT this has to come after ../config.h
#include "kcalctype.h"

#define STACK_SIZE	100
#define PRECEDENCE_INCR	20

#define FUNC_NULL		0
#define FUNC_OR			1
#define FUNC_XOR		2
#define FUNC_AND		3
#define FUNC_LSH		4
#define FUNC_RSH		5
#define FUNC_ADD		6
#define FUNC_SUBTRACT	7
#define FUNC_MULTIPLY	8
#define FUNC_DIVIDE		9
#define FUNC_MOD		10
#define FUNC_POWER		11
#define FUNC_PWR_ROOT	12
#define FUNC_INTDIV		13

#define		HEX_SIZE	sizeof(KCALC_LONG)*2
#define		OCT_SIZE	11
#define		DEC_SIZE	19
#define		BIN_SIZE	32

#define		DSP_SIZE	50 // 25

#define		POS_ZERO	 1e-19L	 /* What we consider zero is */
#define		NEG_ZERO	-1e-19L	 /* anything between these two */

#define DISPLAY_AMOUNT display_data.s_item_data.item_amount


typedef	CALCAMNT	(*Arith)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Prcnt)(CALCAMNT, CALCAMNT);
typedef	CALCAMNT	(*Trig)(CALCAMNT);

typedef enum _last_input_type
{
	DIGIT = 1,
	OPERATION = 2,
	RECALL = 3,
	PASTE = 4
} last_input_type;

typedef enum _num_base
{
	NB_BINARY = 2,
	NB_OCTAL = 8,
	NB_DECIMAL = 10,
	NB_HEX = 16
} num_base;

typedef enum _angle_type
{
	ANG_DEGREE = 0,
	ANG_RADIAN = 1,
	ANG_GRADIENT = 2
} angle_type;

typedef enum _item_type
{
	ITEM_FUNCTION,
	ITEM_AMOUNT
} item_type;

typedef struct _func_data
{
	int item_function;
	int item_precedence;
} func_data;

typedef	union  _item_data
{
	CALCAMNT	item_amount;	// an amount
	func_data	item_func_data;	// or a function
} item_data;

typedef struct _item_contents
{
	item_type	s_item_type;	// a type flag
	item_data	s_item_data; 	// and data
} item_contents;

typedef struct stack_item *stack_ptr;

typedef struct stack_item
{
	// Contents of an item on the input stack

	stack_ptr prior_item;		// Pointer to prior item
	stack_ptr prior_type;		// Pointer to prior type
	item_contents item_value;	// The value of the item
} stack_item;

void 			InitStack();
void 			PushStack(item_contents *add_item);
//int			UpdateStack(int run_precedence);
item_contents	*PopStack(item_type rqstd_type);
item_contents	*TopOfStack();
item_contents	*TopTypeStack(item_type rqstd_type);


typedef struct _DefStruct
{
	QColor forecolor;
	QColor backcolor;
	QColor numberButtonColor;
	QColor functionButtonColor;
	QColor hexButtonColor;
	QColor memoryButtonColor;
	QColor operationButtonColor;

	int precision;
	int fixedprecision;
	int style;
	bool fixed;
	bool beep;
	QFont font;
} DefStruct;


class QtCalculator : public KDialog
{
    Q_OBJECT

public:
	QtCalculator(QWidget *parent = 0, const char *name = 0);
	~QtCalculator();

private:
	virtual bool eventFilter( QObject *o, QEvent *e );
	void updateGeometry();

	void keyPressEvent( QKeyEvent *e );
	void keyReleaseEvent( QKeyEvent *e );
	void closeEvent( QCloseEvent *e );
	void writeSettings();
	void readSettings();
	void set_precision();
	void set_display_font();
	void set_style();
	void history_next();
	void history_prev();
	void ComputeMean();
	void ComputeSin();
	void ComputeStd();
	void ComputeCos();
	void ComputeMedean();
	void ComputeTan();

	int UpdateStack(int run_precedence);
protected slots:
    void configurationChanged(const DefStruct &state);
    void set_colors();
    void display_selected();
    void invertColors();
    void quitCalc();
    void selection_timed_out();
    void clear_buttons();
    void clear_status_label();
    void setStatusLabel(const QString&);
    void EnterDigit(int data);
    void SubtractDigit();
    void EnterDecimal();
    void EnterStackFunction(int data);
    void EnterNegate();
    void EnterOpenParen();
    void EnterCloseParen();
    void EnterRecip();
    void EnterInt();
    void EnterFactorial();
    void EnterSquare();
    void EnterNotCmp();
    void EnterHyp();
    void EnterPercent();
    void EnterLogr();
    void EnterLogn();
    void Deg_Selected();
    void Rad_Selected();
    void Gra_Selected();
    void Base_Selected(int number);
    void Hex_Selected();
    void Dec_Selected();
    void Oct_Selected();
    void Bin_Selected();
    void SetInverse();
    void EnterEqual();
    void Clear();
    void ClearAll();
    void RefreshCalculator();
    void InitializeCalculator();
    void UpdateDisplay();
    void ExecSin();
    void ExecCos();
    void ExecTan();
    void button0();
    void button1();
    void button2();
    void button3();
    void button4();
    void button5();
    void button6();
    void button7();
    void button8();
    void button9();
    void buttonA();
    void buttonB();
    void buttonC();
    void buttonD();
    void buttonE();
    void buttonF();
    void base_selected(int number);
    void angle_selected(int number);
    void Or();
    void And();
    void Shift();
    void Plus();
    void Minus();
    void Multiply();
    void Divide();
    void Mod();
    void Power();
    void EE();
    void MR();
    void Mplusminus();
    void MC();
    void EEtoggled(bool myboolean);
    void pbinvtoggled(bool myboolean);
    void pbMRtoggled(bool myboolean);
    void pbAtoggled(bool myboolean);
    void pbSintoggled(bool myboolean);
    void pbplusminustoggled(bool myboolean);
    void pbMplusminustoggled(bool myboolean);
    void pbBtoggled(bool myboolean);
    void pbCostoggled(bool myboolean);
    void pbrecitoggled(bool myboolean);
    void pbCtoggled(bool myboolean);
    void pbTantoggled(bool myboolean);
    void pbfactorialtoggled(bool myboolean);
    void pbDtoggled(bool myboolean);
    void pblogtoggled(bool myboolean);
    void pbsquaretoggled(bool myboolean);
    void pbEtoggled(bool myboolean);
    void pblntoggled(bool myboolean);
    void pbpowertoggled(bool myboolean);
    void pbFtoggled(bool myboolean);
    void pbMCtoggled(bool myboolean);
    void pbCleartoggled(bool myboolean);
    void pbACtoggled(bool myboolean);
    void pb7toggled(bool myboolean);
    void pb8toggled(bool myboolean);
    void pb9toggled(bool myboolean);
    void pbparenopentoggled(bool myboolean);
    void pbparenclosetoggled(bool myboolean);
    void pbandtoggled(bool myprivateboolean);
    void pb4toggled(bool myboolean);
    void pb5toggled(bool myboolean);
    void pb6toggled(bool myboolean);
    void pbXtoggled(bool myboolean);
    void pbdivisiontoggled(bool myboolean);
    void pbortoggled(bool myboolean);
    void pb1toggled(bool myboolean);
    void pb2toggled(bool myboolean);
    void pb3toggled(bool myboolean);
    void pbplustoggled(bool myboolean);
    void pbminustoggled(bool myboolean);
    void pbshifttoggled(bool privatemyboolean);
    void pbperiodtoggled(bool myboolean);
    void pb0toggled(bool myboolean);
    void pbequaltoggled(bool myboolean);
    void pbpercenttoggled(bool myboolean);
    void pbnegatetoggled(bool myboolean);
    void pbmodtoggled(bool myboolean);
    void pbhyptoggled(bool myboolean);
    void configclicked();

private:
     DefStruct kcalcdefaults;

private:
	static int cvb(char *out_str, KCALC_LONG amount, int max_digits);

public:
	static CALCAMNT ExecOr(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecXor(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecAnd(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecLsh(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecRsh(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecAdd(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecSubtract(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecMultiply(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecDivide(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecMod(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecPower(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecPwrRoot(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecIntDiv(CALCAMNT left_op, CALCAMNT right_op);
        static CALCAMNT ExecAddP(CALCAMNT left_op, CALCAMNT right_op);
        static CALCAMNT ExecSubP(CALCAMNT left_op, CALCAMNT right_op);
        static CALCAMNT ExecMultiplyP(CALCAMNT left_op, CALCAMNT right_op);
        static CALCAMNT ExecDivideP(CALCAMNT left_op, CALCAMNT right_op);
	static CALCAMNT ExecFunction(CALCAMNT left_op, int function, CALCAMNT right_op);

public:
	static CALCAMNT Deg2Rad(CALCAMNT x)	{ return (((2L * pi) / 360L) * x); }
	static CALCAMNT Gra2Rad(CALCAMNT x)	{ return ((pi / 200L) * x); }
	static CALCAMNT Rad2Deg(CALCAMNT x)	{ return ((360L / (2L * pi)) * x); }
	static CALCAMNT Rad2Gra(CALCAMNT x)	{ return ((200L / pi) * x); }

private:
	bool inverse;
	bool hyp_mode;
	bool eestate;
	bool refresh_display;
	int	display_size;
	int	angle_mode;
	int input_limit;
	int input_count;
	int decimal_point;
	int precedence_base;
	num_base current_base;
	CALCAMNT memory_num;
	last_input_type last_input;

public:
	static const CALCAMNT pi;

private:
	char display_str[DSP_SIZE + 1];

private:
	std::vector<CALCAMNT> history_list;
	int history_index;


private:
    QWidget *mSmallPage;
    QWidget *mLargePage;

    KPushButton *mConfigButton;
    KPushButton *mHelpButton;

    QTimer* selection_timer;
    QLabel* statusINVLabel;
    QLabel* statusHYPLabel;
    QLabel* statusERRORLabel;
    DLabel*		calc_display;
    QRadioButton*	anglebutton[3];
    QRadioButton*	basebutton[4];
    QPushButton*        pbhyp;
    QPushButton* 	pbEE;
    QPushButton* 	pbinv;
    QPushButton* 	pbMR;
    QPushButton* 	pbA;
    QPushButton* 	pbSin;
    QPushButton* 	pbplusminus;
    QPushButton* 	pbMplusminus;
    QPushButton* 	pbB;
    QPushButton* 	pbCos;
    QPushButton* 	pbreci;
    QPushButton* 	pbC;
    QPushButton* 	pbTan;
    QPushButton* 	pbfactorial;
    QPushButton* 	pbD;
    QPushButton* 	pblog;
    QPushButton* 	pbsquare;
    QPushButton* 	pbE;
    QPushButton* 	pbln;
    QPushButton* 	pbpower;
    QPushButton* 	pbF;
    QPushButton* 	pbMC;
    QPushButton* 	pbClear;
    QPushButton* 	pbAC;
    QPushButton* 	pb7;
    QPushButton* 	pb8;
    QPushButton* 	pb9;
    QPushButton* 	pbparenopen;
    QPushButton* 	pbparenclose;
    QPushButton* 	pband;
    QPushButton* 	pb4;
    QPushButton* 	pb5;
    QPushButton* 	pb6;
    QPushButton* 	pbX;
    QPushButton* 	pbdivision;
    QPushButton* 	pbor;
    QPushButton* 	pb1;
    QPushButton* 	pb2;
    QPushButton* 	pb3;
    QPushButton* 	pbplus;
    QPushButton* 	pbminus;
    QPushButton* 	pbshift;
    QPushButton* 	pbperiod;
    QPushButton* 	pb0;
    QPushButton* 	pbequal;
    QPushButton* 	pbpercent;
    QPushButton* 	pbnegate;
    QPushButton* 	pbmod;
    QButtonGroup*	angle_group;

    KHelpMenu *mHelpMenu;

    QPtrList<QPushButton> mNumButtonList;
    QPtrList<QPushButton> mFunctionButtonList;
    QPtrList<QPushButton> mHexButtonList;
    QPtrList<QPushButton> mMemButtonList;
    QPtrList<QPushButton> mOperationButtonList;

    bool			key_pressed;
    int				mInternalSpacing;
    KStats			stats;
    QListBox		*paper;
    QTimer			*status_timer;
    ConfigureDialog	*mConfigureDialog;
};

#endif  //QTCLAC_H
