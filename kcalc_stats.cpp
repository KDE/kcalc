/*
    SPDX-FileCopyrightText: 2001-2013 Evan Teran <evan.teran@gmail.com>
    SPDX-FileCopyrightText: 1996-2000 Bernd Johannes Wuebben <wuebben@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcalc_stats.h"

//------------------------------------------------------------------------------
// Name: KStats
// Desc: constructor
//------------------------------------------------------------------------------
KStats::KStats() = default;

//------------------------------------------------------------------------------
// Name: ~KStats
// Desc: destructor
//------------------------------------------------------------------------------
KStats::~KStats() = default;

//------------------------------------------------------------------------------
// Name: clearAll
// Desc: empties the data set
//------------------------------------------------------------------------------
void KStats::clearAll()
{
    m_data.clear();
}

//------------------------------------------------------------------------------
// Name: enterData
// Desc: adds an item to the data set
//------------------------------------------------------------------------------
void KStats::enterData(const KNumber &data)
{
    this->m_data.push_back(data);
}

//------------------------------------------------------------------------------
// Name: clearLast
// Desc: removes the last item from the data set
//------------------------------------------------------------------------------
void KStats::clearLast()
{
    if (!m_data.isEmpty()) {
        m_data.pop_back();
    }
}

//------------------------------------------------------------------------------
// Name: sum
// Desc: calculates the SUM of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::sum() const
{
    KNumber result = KNumber::Zero;
    for (const KNumber &x : std::as_const(m_data)) {
        result += x;
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: median
// Desc: calculates the MEDIAN of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::median()
{
    KNumber result = KNumber::Zero;
    size_t index = 0;

    unsigned int bound = count();

    if (bound == 0) {
        m_errorFlag = true;
        return KNumber::Zero;
    }

    if (bound == 1) {
        return m_data.at(0);
    }

    // need to copy data_-list, because sorting afterwards
    QList<KNumber> tmpData(m_data);
    std::sort(tmpData.begin(), tmpData.end());

    if (bound & 1) { // odd
        index = (bound - 1) / 2 + 1;
        result = tmpData.at(index - 1);
    } else { // even
        index = bound / 2;
        result = ((tmpData.at(index - 1)) + (tmpData.at(index))) / KNumber(2);
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: std_kernel
// Desc: calculates the STD Kernel of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::stdKernel()
{
    KNumber result = KNumber::Zero;
    const KNumber meanValue = mean();

    if (meanValue.type() != KNumber::TypeError) {
        for (const KNumber &x : std::as_const(m_data)) {
            result += (x - meanValue) * (x - meanValue);
        }
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: sum_of_squares
// Desc: calculates the SUM of all values in the data set (each squared)
//------------------------------------------------------------------------------
KNumber KStats::sumOfSquares() const
{
    KNumber result = KNumber::Zero;

    for (const KNumber &x : std::as_const(m_data)) {
        result += (x * x);
    }

    return result;
}

//------------------------------------------------------------------------------
// Name: mean
// Desc: calculates the MEAN of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::mean()
{
    if (m_data.isEmpty()) {
        m_errorFlag = true;
        return KNumber::Zero;
    }

    return (sum() / KNumber(count()));
}

//------------------------------------------------------------------------------
// Name: std
// Desc: calculates the STANDARD DEVIATION of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::std()
{
    if (m_data.isEmpty()) {
        m_errorFlag = true;
        return KNumber::Zero;
    }

    return (stdKernel() / KNumber(count())).sqrt();
}

//------------------------------------------------------------------------------
// Name: sample_std
// Desc: calculates the SAMPLE STANDARD DEVIATION of all values in the data set
//------------------------------------------------------------------------------
KNumber KStats::sampleStd()
{
    KNumber result = KNumber::Zero;

    if (count() < 2) {
        m_errorFlag = true;
        return KNumber::Zero;
    }

    result = (stdKernel() / KNumber(count() - 1)).sqrt();

    return result;
}

//------------------------------------------------------------------------------
// Name: count
// Desc: returns the amount of values in the data set
//------------------------------------------------------------------------------
int KStats::count() const
{
    return m_data.size();
}

//------------------------------------------------------------------------------
// Name: error
// Desc: returns the error state AND clears it
//------------------------------------------------------------------------------
bool KStats::error()
{
    bool value = m_errorFlag = false;
    m_errorFlag = false;
    return value;
}
