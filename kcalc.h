/*
    $id: kcalc.h,v 1.47 2003/03/19 01:35:11 bmeyer Exp $

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef QTCALC_H
#define QTCALC_H

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
#include <kmainwindow.h>
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
#define FUNC_SUBTRACT		7
#define FUNC_MULTIPLY		8
#define FUNC_DIVIDE		9
#define FUNC_MOD		10
#define FUNC_POWER		11
#define FUNC_PWR_ROOT		12
#define FUNC_INTDIV		13

#define		HEX_SIZE	sizeof(KCALC_LONG)*2
#define		OCT_SIZE	11
#define		DEC_SIZE	19
#define		BIN_SIZE	32

#define		DSP_SIZE	50 // 25

#define		POS_ZERO	 1e-19L	 /* What we consider zero is */
#define		NEG_ZERO	-1e-19L	 /* anything between these two */

#define DISPLAY_AMOUNT display_data


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

typedef struct _func_data
{
	int item_function;
	int item_precedence;
} func_data;


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
	bool capres;
	QFont font;
} DefStruct;


class QtCalculator : public KMainWindow
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
    void slotNumberclicked(int number_clicked);
    void slotEEclicked(void);
    void slotInvtoggled(bool myboolean);
    void slotMRclicked(void);
    void slotSinclicked(void);
    void slotPlusMinusclicked(void);
    void slotMPlusMinusclicked(void);
    void slotCosclicked(void);
    void slotReciclicked(void);
    void slotTanclicked(void);
    void slotFactorialclicked(void);
    void slotLogclicked(void);
    void slotSquareclicked(void);
    void slotLnclicked(void);
    void slotPowerclicked(void);
    void slotMCclicked(void);
    void slotClearclicked(void);
    void slotACclicked(void);
    void slotParenOpenclicked(void);
    void slotParenCloseclicked(void);
    void slotANDclicked(void);
    void slotXclicked(void);
    void slotDivisionclicked(void);
    void slotORclicked(void);
    void slotPlusclicked(void);
    void slotMinusclicked(void);
    void slotShiftclicked(void);
    void slotPeriodclicked(void);
    void slotEqualclicked(void);
    void slotPercentclicked(void);
    void slotNegateclicked(void);
    void slotModclicked(void);
    void slotHyptoggled(bool myboolean);
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
	QValueVector<CALCAMNT> history_list;
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
    QPushButton*        pbHyp;
    QPushButton* 	pbEE;
    QPushButton* 	pbInv;
    QPushButton* 	pbMR;
    QPushButton* 	pbSin;
    QPushButton* 	pbPlusMinus;
    QPushButton* 	pbMPlusMinus;
    QPushButton* 	pbCos;
    QPushButton* 	pbReci;
    QPushButton* 	pbTan;
    QPushButton* 	pbFactorial;
    QPushButton* 	pbLog;
    QPushButton* 	pbSquare;
    QPushButton* 	pbLn;
    QPushButton* 	pbPower;
    QPushButton* 	pbMC;
    QPushButton* 	pbClear;
    QPushButton* 	pbAC;
    QPushButton* 	pbParenOpen;
    QPushButton* 	pbParenClose;
    QPushButton* 	pbAND;
    QPushButton* 	pbX;
    QPushButton* 	pbDivision;
    QPushButton* 	pbOR;
    QPushButton* 	pbPlus;
    QPushButton* 	pbMinus;
    QPushButton* 	pbShift;
    QPushButton* 	pbPeriod;
    QPushButton* 	pbEqual;
    QPushButton* 	pbPercent;
    QPushButton* 	pbNegate;
    QPushButton* 	pbMod;
    QButtonGroup*	angle_group;
    // NumButtonGroup: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup*       NumButtonGroup;

    KHelpMenu *mHelpMenu;

    QPtrList<QPushButton> mFunctionButtonList;
    QPtrList<QPushButton> mMemButtonList;
    QPtrList<QPushButton> mOperationButtonList;

    bool			key_pressed;
    int				mInternalSpacing;
    KStats			stats;
    QTimer			*status_timer;
    ConfigureDialog	*mConfigureDialog;
};

#endif  //QTCLAC_H
