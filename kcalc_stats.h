/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "knumber.h"
#include <QList>

class KStats
{
public:
    KStats();
    ~KStats();

public:
    void clearAll();
    void enterData(const KNumber &data);
    void clearLast();
    KNumber sum() const;
    KNumber sumOfSquares() const;
    KNumber mean();
    KNumber median();
    KNumber stdKernel();
    KNumber std();
    KNumber sampleStd();
    int count() const;
    bool error();

private:
    QList<KNumber> m_data;
    bool m_errorFlag = false;
};
