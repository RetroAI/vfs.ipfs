/*
 *      Copyright (C) 2017 squishyhuman
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "libXBMC_addon.h"

#define KODI_TEMP_DIR  "special://temp/"

ADDON::CHelper_libXBMC_addon *KODI = nullptr;

extern "C" {

#include "kodi_vfs_dll.h"

static std::string URLEncode(const std::string& strURLData)
{
  std::string strResult;

  /* wonder what a good value is here is, depends on how often it occurs */
  strResult.reserve( strURLData.length() * 2 );

  for (size_t i = 0; i < strURLData.size(); ++i)
  {
    const char kar = strURLData[i];

    // Don't URL encode "-_.!()" according to RFC1738
    //! @todo Update it to "-_.~" after Gotham according to RFC3986
    if (std::isalnum(kar) || kar == '-' || kar == '.' || kar == '_' || kar == '!' || kar == '(' || kar == ')')
      strResult.push_back(kar);
    else
    {
      char temp[128];
      sprintf(temp,"%%%2.2X", (unsigned int)((unsigned char)kar));
      strResult += temp;
    }
  }

  return strResult;
}

//-- Create -------------------------------------------------------------------
// Called on load. Addon should fully initalize or return error status
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!KODI)
    KODI = new ADDON::CHelper_libXBMC_addon;

  if (!KODI->RegisterMe(hdl))
  {
    delete KODI;
    KODI = nullptr;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  return ADDON_STATUS_OK;
}

//-- Stop ---------------------------------------------------------------------
// This dll must cease all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Stop()
{
}

//-- Destroy ------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Destroy()
{
  KODI = nullptr;
}

//-- HasSettings --------------------------------------------------------------
// Returns true if this add-on use settings
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
bool ADDON_HasSettings()
{
  return false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

//-- GetSettings --------------------------------------------------------------
// Return the settings for XBMC to display
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

//-- FreeSettings --------------------------------------------------------------
// Free the settings struct passed from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------

void ADDON_FreeSettings()
{
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void* value)
{
  return ADDON_STATUS_OK;
}

//-- Announce -----------------------------------------------------------------
// Receive announcements from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

struct IpfsContext
{
  std::string cachedir;
  std::string ipfspath;
  std::string password;
  std::string pathinipfs;
  int8_t fileoptions;
  int64_t size;
  void* file;
  int64_t fileposition;
  int64_t bufferstart;
  bool seekable;

  IpfsContext() :
    fileoptions(0),
    size(0),
    file(nullptr),
    fileposition(0),
    bufferstart(0),
    seekable(true)
  {
  }

  void Init(VFSURL* url)
  {
    cachedir = KODI_TEMP_DIR;
    ipfspath = url->hostname ? url->hostname : "";
    password = url->password ? url->password : "";
    pathinipfs = url->filename ? url->filename : "";

    std::vector<std::string> options;
    std::string strOptions(url->options ? url->options : "");
    /*
    if (!strOptions.empty())
      CIpfsManager::Tokenize(strOptions.substr(1), options, "&");
    */

    fileoptions = 0;
    for (const auto& option : options)
    {
      size_t iEqual = option.find('=');
      if(iEqual != std::string::npos)
      {
        std::string strOption = option.substr(0, iEqual);
        std::string strValue = option.substr(iEqual+1);

        if (strOption == "flags")
          fileoptions = atoi(strValue.c_str());
        else if (strOption == "cache")
          cachedir = strValue;
      }
    }
  }

  bool OpenInArchive()
  {
    return false;
  }

  void CleanUp()
  {
  }
};

void* Open(VFSURL* url)
{
  return nullptr;
}

ssize_t Read(void* context, void* lpBuf, size_t uiBufSize)
{
  IpfsContext* ctx = (IpfsContext*)context;

  if (ctx->file)
    return KODI->ReadFile(ctx->file,lpBuf,uiBufSize);

  if (ctx->fileposition >= GetLength(context)) // we are done
    return 0;

  return 0;
}

bool Close(void* context)
{
  IpfsContext* ctx = (IpfsContext*)context;
  if (!ctx)
    return true;

  if (ctx->file)
  {
    KODI->CloseFile(ctx->file);
  }
  else
  {
    ctx->CleanUp();
  }
  delete ctx;

  return true;
}

int64_t GetLength(void* context)
{
  IpfsContext* ctx = (IpfsContext*)context;

  if (ctx->file)
    return KODI->GetFileLength(ctx->file);

  return ctx->size;
}

//*********************************************************************************************
int64_t GetPosition(void* context)
{
  IpfsContext* ctx = (IpfsContext*)context;
  if (ctx->file)
    return KODI->GetFilePosition(ctx->file);

  return ctx->fileposition;
}


int64_t Seek(void* context, int64_t iFilePosition, int iWhence)
{
  IpfsContext* ctx = (IpfsContext*)context;
  if (!ctx->seekable)
    return -1;

  if (ctx->file)
    return KODI->SeekFile(ctx->file, iFilePosition, iWhence);

  switch (iWhence)
  {
    case SEEK_CUR:
      if (iFilePosition == 0)
        return ctx->fileposition; // happens sometimes

      iFilePosition += ctx->fileposition;
      break;
    case SEEK_END:
      if (iFilePosition == 0) // do not seek to end
      {
        ctx->fileposition = GetLength(context);
        ctx->bufferstart = GetLength(context);

        return GetLength(context);
      }

      iFilePosition += GetLength(context);
      break;
    case SEEK_SET:
      break;
    default:
      return -1;
  }

  if (iFilePosition > GetLength(context))
    return -1;

  if (iFilePosition == ctx->fileposition) // happens a lot
    return ctx->fileposition;

  ctx->fileposition = iFilePosition;

  return ctx->fileposition;
}

bool Exists(VFSURL* url)
{
  IpfsContext ctx;
  ctx.Init(url);
  
  // First step:
  // Make sure that the archive exists in the filesystem.
  if (!KODI->FileExists(ctx.ipfspath.c_str(), false)) 
    return false;

  return false;
}

int Stat(VFSURL* url, struct __stat64* buffer)
{
  memset(buffer, 0, sizeof(struct __stat64));
  IpfsContext* ctx = (IpfsContext*)Open(url);
  if (ctx)
  {
    buffer->st_size = ctx->size;
    buffer->st_mode = S_IFREG;
    Close(ctx);
    errno = 0;
    return 0;
  }

  Close(ctx);
  if (DirectoryExists(url))
  {
    buffer->st_mode = S_IFDIR;
    return 0;
  }

  errno = ENOENT;
  return -1;
}

void ClearOutIdle()
{
}

void DisconnectAll()
{
}

bool DirectoryExists(VFSURL* url)
{
  VFSDirEntry* dir;
  int num_items;
  void* ctx = GetDirectory(url, &dir, &num_items, NULL);
  if (ctx)
  {
    FreeDirectory(ctx);
    return true;
  }

  return false;
}

void* GetDirectory(VFSURL* url, VFSDirEntry** items,
                   int* num_items, VFSCallbacks* callbacks)
{
  std::string strPath(url->url);
  size_t pos;
  if ((pos=strPath.find("?")) != std::string::npos)
    strPath.erase(strPath.begin()+pos, strPath.end());

  // the IPFS code depends on things having a "\" at the end of the path
  if (strPath[strPath.size()-1] != '/')
    strPath += '/';

  std::string strArchive = url->hostname;
  std::string strOptions = url->options;
  std::string strPathInArchive = url->filename;

  std::vector<VFSDirEntry>* itms = new std::vector<VFSDirEntry>;
  return itms;
}

void FreeDirectory(void* items)
{
  std::vector<VFSDirEntry>& ctx = *static_cast<std::vector<VFSDirEntry>>(items);
  for (size_t i = 0; i < ctx.size(); ++i)
  {
    free(ctx[i].label);
    for (size_t j = 0; j < ctx[i].num_props; ++j)
    {
      free(ctx[i].properties[j].name);
      free(ctx[i].properties[j].val);
    }
    delete ctx[i].properties;
    free(ctx[i].path);
  }
  delete &ctx;
}

bool CreateDirectory(VFSURL* url)
{
  return false;
}

bool RemoveDirectory(VFSURL* url)
{
  return false;
}

int Truncate(void* context, int64_t size)
{
  return -1;
}

ssize_t Write(void* context, const void* lpBuf, size_t uiBufSize)
{
  return -1;
}

bool Delete(VFSURL* url)
{
  return false;
}

bool Rename(VFSURL* url, VFSURL* url2)
{
  return false;
}

void* OpenForWrite(VFSURL* url, bool bOverWrite)
{ 
  return NULL;
}

void* ContainsFiles(VFSURL* url, VFSDirEntry** items, int* num_items, char* rootpath)
{
  std::vector<VFSDirEntry>* itms = new std::vector<VFSDirEntry>();
  return itms;
}

int GetStartTime(void* ctx)
{
  return 0;
}

int GetTotalTime(void* ctx)
{
  return 0;
}

bool NextChannel(void* context, bool preview)
{
  return false;
}

bool PrevChannel(void* context, bool preview)
{
  return false;
}

bool SelectChannel(void* context, unsigned int uiChannel)
{
  return false;
}

bool UpdateItem(void* context)
{
  return false;
}

int GetChunkSize(void* context)
{
  return 0;
}

}
