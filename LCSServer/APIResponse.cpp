#include "APIResponse.h"

APIResponse::APIResponse(void)
    : m_returnCode("200 OK"), m_contentType("text/html")
{

}

void APIResponse::copy(const APIResponse &other)
{
    m_contentType = other.m_contentType;
    m_returnCode = other.m_returnCode;
    m_payload = other.m_payload;
}
