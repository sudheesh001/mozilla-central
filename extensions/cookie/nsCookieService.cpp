/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#define NS_IMPL_IDS

#include "nsCOMPtr.h"
#include "nsIFactory.h"
#include "nsIServiceManager.h"
#include "nsICookieService.h"
#include "nsCookieHTTPNotify.h"
#include "nsINetModuleMgr.h" 
#include "nsIEventQueueService.h"
#include "nsCRT.h"
#include "nsCookie.h"
#include "nsIModule.h"
#include "nsIGenericFactory.h"

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID); 
static NS_DEFINE_IID(kICookieServiceIID, NS_ICOOKIESERVICE_IID);

static NS_DEFINE_CID(kNetModuleMgrCID, NS_NETMODULEMGR_CID); 
static NS_DEFINE_IID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);

////////////////////////////////////////////////////////////////////////////////

class nsCookieService : public nsICookieService {
public:

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsICookieService
  static nsresult GetCookieService(nsICookieService** aCookieService);

  NS_IMETHOD GetCookieString(nsIURI *aURL, nsString& aCookie);
  NS_IMETHOD SetCookieString(nsIURI *aURL, const nsString& aCookie);
  NS_IMETHOD Cookie_RemoveAllCookies(void);
  NS_IMETHOD Cookie_CookieViewerReturn(nsAutoString results);
  NS_IMETHOD Cookie_GetCookieListForViewer(nsString& aCookieList);
  NS_IMETHOD Cookie_GetPermissionListForViewer(nsString& aPermissionList);

  nsCookieService();
  virtual ~nsCookieService(void);
  
protected:
    
private:
  nsIHTTPNotify *mCookieHTTPNotify;
  nsresult Init();
};

static nsCookieService* gCookieService = nsnull; // The one-and-only CookieService

////////////////////////////////////////////////////////////////////////////////
// nsCookieService Implementation

NS_IMPL_ISUPPORTS(nsCookieService, kICookieServiceIID);

NS_EXPORT nsresult NS_NewCookieService(nsICookieService** aCookieService) {
  return nsCookieService::GetCookieService(aCookieService);
}

nsCookieService::nsCookieService() {
  NS_INIT_REFCNT();
  mCookieHTTPNotify = nsnull;
  Init();
}

nsCookieService::~nsCookieService(void) {
  NS_IF_RELEASE(mCookieHTTPNotify);
  gCookieService = nsnull;
}

nsresult nsCookieService::GetCookieService(nsICookieService** aCookieService) {
  if (! gCookieService) {
    nsCookieService* it = new nsCookieService();
    if (! it) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    gCookieService = it;
  }
  NS_ADDREF(gCookieService);
  *aCookieService = gCookieService;
  return NS_OK;
}

nsresult nsCookieService::Init() {
  nsresult rv;
  NS_WITH_SERVICE(nsINetModuleMgr, pNetModuleMgr, kNetModuleMgrCID, &rv); 
  if (NS_FAILED(rv)) return rv;

  NS_WITH_SERVICE(nsIEventQueueService, eventQService, kEventQueueServiceCID, &rv); 
  if (NS_FAILED(rv)) return rv;
  rv = eventQService->CreateThreadEventQueue();
  if (NS_FAILED(rv)) return rv;
  
  if (NS_FAILED(rv = NS_NewCookieHTTPNotify(&mCookieHTTPNotify))) {
    return rv;
  }
  rv = pNetModuleMgr->RegisterModule(NS_NETWORK_MODULE_MANAGER_HTTP_REQUEST_PROGID, mCookieHTTPNotify);
  if (NS_FAILED(rv)) return rv;

  rv = pNetModuleMgr->RegisterModule(NS_NETWORK_MODULE_MANAGER_HTTP_RESPONSE_PROGID, mCookieHTTPNotify);
  if (NS_FAILED(rv)) return rv;

  COOKIE_RegisterCookiePrefCallbacks();
  COOKIE_ReadCookies();     
  return rv;
}


NS_IMETHODIMP
nsCookieService::GetCookieString(nsIURI *aURL, nsString& aCookie) {
  char *spec = NULL;
  nsresult result = aURL->GetSpec(&spec);
  NS_ASSERTION(result == NS_OK, "deal with this");
  char *cookie = COOKIE_GetCookie((char *)spec);
  if (nsnull != cookie) {
    aCookie.SetString(cookie);
    nsCRT::free(cookie);
  } else {
    aCookie.SetString("");
  }
  nsCRT::free(spec);
  return NS_OK;
}

NS_IMETHODIMP
nsCookieService::SetCookieString(nsIURI *aURL, const nsString& aCookie) {
  char *spec = NULL;
  nsresult result = aURL->GetSpec(&spec);
  NS_ASSERTION(result == NS_OK, "deal with this");
  char *cookie = aCookie.ToNewCString();
  COOKIE_SetCookieString((char *)spec, cookie);
  nsCRT::free(spec);
  nsCRT::free(cookie);
  return NS_OK;
}

NS_IMETHODIMP nsCookieService::Cookie_RemoveAllCookies(void) {
  ::COOKIE_RemoveAllCookies();
  return NS_OK;
}

NS_IMETHODIMP nsCookieService::Cookie_CookieViewerReturn(nsAutoString results) {
  ::COOKIE_CookieViewerReturn(results);
  return NS_OK;
}

NS_IMETHODIMP nsCookieService::Cookie_GetCookieListForViewer(nsString& aCookieList) {
  ::COOKIE_GetCookieListForViewer(aCookieList);
  return NS_OK;
}

NS_IMETHODIMP nsCookieService::Cookie_GetPermissionListForViewer(nsString& aPermissionList) {
  ::COOKIE_GetPermissionListForViewer(aPermissionList);
  return NS_OK;
}


//----------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Define the contructor function for the objects
//
// NOTE: This creates an instance of objects by using the default constructor
//
// NS_GENERIC_FACTORY_CONSTRUCTOR(nsCookieService)
static NS_IMETHODIMP    
CreateNewCookieService(nsISupports* aOuter, REFNSIID aIID, void **aResult)
{                                                                    
    if (!aResult) {                                              
        return NS_ERROR_INVALID_POINTER; 
    }                            
    if (aOuter) {          
        *aResult = nsnull;
        return NS_ERROR_NO_AGGREGATION;
    }                                 
    nsICookieService* inst = nsnull;                   
    nsresult rv = NS_NewCookieService(&inst); 
    if (NS_FAILED(rv)) {       
        *aResult = nsnull;     
        return rv;          
    } 
    if(!inst) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    rv = inst->QueryInterface(aIID, aResult); 
    if (NS_FAILED(rv)) { 
        *aResult = nsnull; 
    }                      
    NS_RELEASE(inst);             /* get rid of extra refcnt */ 
    return rv;                
}

////////////////////////////////////////////////////////////////////////
// Define a table of CIDs implemented by this module along with other
// information like the function to create an instance, progid, and
// class name.
//
static nsModuleComponentInfo components[] = {
    { "CookieService", NS_COOKIESERVICE_CID,
      "component://netscape/cookie", CreateNewCookieService, },	// XXX Singleton
    { "CookieHTTPNotifyService", NS_COOKIEHTTPNOTIFY_CID,
      "component://netscape/cookie-http-notify", CreateNewCookieService, },
};

////////////////////////////////////////////////////////////////////////
// Implement the NSGetModule() exported function for your module
// and the entire implementation of the module object.
//
NS_IMPL_NSGETMODULE("nsCookieModule", components)
