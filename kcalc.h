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

/*
  Kcalc basically consist of a class for the GUI (here) and one for
  the mathematics core (kcalc_core.h). When for example '+' is pressed
  one sends the contents of the Display and the '+' to the core via
  "core.Plus(DISPLAY_AMOUNT);".  The display is updated via
  "UpdateDisplay(bool)". The argument says if the amount to display is
  already stored in DISPLAY_AMOUNT (e.g. user is typing in a number)
  or if it should be asked form the core(some function was computed).
 */

#ifdef HAVE_CONFIG_H
	#include "../config.h"
#endif

// IMPORTANT this has to come after ../config.h
#include "kcalc_core.h"


#define		HEX_SIZE	sizeof(KCALC_LONG)*2
#define		OCT_SIZE	11
#define		DEC_SIZE	19
#define		BIN_SIZE	32

#define		DSP_SIZE	50 // 25

#define DISPLAY_AMOUNT display_data

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

typedef struct _DefStruct
{
	QColor forecolor;
	QColor backcolor;
	QColor numberButtonColor;
	QColor functionButtonColor;
	QColor statButtonColor;
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
	void closeEvent( QCloseEvent *e );
	void writeSettings();
	void readSettings();
	void set_precision();
	void set_display_font();
	void set_style();
	void history_next();
	void history_prev();

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
    void EnterNegate();
    void EnterInt();
    void EnterEqual();
    void ClearAll();
    void RefreshCalculator();
    void UpdateDisplay(bool get_new_data_from_core);
    void base_selected(int number);
    void EE();
    void slotAngleSelected(int number);
    void slotBaseSelected(int number);
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
    void slotStatNumclicked(void);
    void slotStatMeanclicked(void);
    void slotStatStdDevclicked(void);
    void slotStatMedianclicked(void);
    void slotStatDataInputclicked(void);
    void slotStatClearDataclicked(void);
    void slotHyptoggled(bool flag);
    void slotConfigclicked(void);

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

private:
	bool inverse;
	bool hyp_mode;
	bool eestate;
	bool display_error;
	int	display_size;
	int input_limit;
	int input_count;
	int decimal_point;
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
    QPushButton*        pbStatNum;
    QPushButton*        pbStatMean;
    QPushButton*        pbStatStdDev;
    QPushButton*        pbStatMedian;
    QPushButton*        pbStatDataInput;
    QPushButton*        pbStatClearData;
    QButtonGroup*	angle_group;
    // NumButtonGroup: 0-9 = digits, 0xA-0xF = hex-keys
    QButtonGroup*       NumButtonGroup;

    KHelpMenu *mHelpMenu;

    QPtrList<QPushButton> mFunctionButtonList;
    QPtrList<QPushButton> mStatButtonList;
    QPtrList<QPushButton> mMemButtonList;
    QPtrList<QPushButton> mOperationButtonList;

    int				mInternalSpacing;
    QTimer			*status_timer;
    ConfigureDialog	*mConfigureDialog;

    CALCAMNT	display_data;

    CalcEngine core;
};

#endif  //QTCALC_H
