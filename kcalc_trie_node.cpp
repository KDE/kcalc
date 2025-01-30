#include "kcalc_trie_node.h"
#include "kcalc_token.h"

#include <QChar>
#include <QHash>

KCalcTrieNode::KCalcTrieNode()
{
    m_isToken = false;
    m_code = KCalcToken::TokenCode::INVALID_TOKEN;
}

KCalcTrieNode::~KCalcTrieNode()
{
    for (auto it = children.constBegin(); it != children.constEnd(); ++it) {
        delete it.value();
    }
}

bool KCalcTrieNode::isToken()
{
    return m_isToken;
}

KCalcToken::TokenCode KCalcTrieNode::code()
{
    return m_code;
}

bool KCalcTrieNode::hasChild(const QChar child)
{
    return children.contains(child);
}

KCalcTrieNode *KCalcTrieNode::child(const QChar child)
{
    return children.value(child);
}

void KCalcTrieNode::addChild(const QChar child)
{
    children.insert(child, new KCalcTrieNode());
}