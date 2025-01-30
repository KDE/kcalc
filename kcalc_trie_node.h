#pragma once

#include "kcalc_token.h"

#include <QChar>
#include <QHash>

class KCalcTrieNode
{
public:
    KCalcTrieNode();
    ~KCalcTrieNode();

    bool hasChild(const QChar child);
    void addChild(const QChar child);
    KCalcTrieNode *child(const QChar child);

    bool isToken();
    KCalcToken::TokenCode code();

    QHash<QChar, KCalcTrieNode *> children;

    bool m_isToken;
    KCalcToken::TokenCode m_code;

private:
    // bool m_isToken;
    // KCalcToken::TokenCode m_code;
};