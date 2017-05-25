char g_last_insertfilename[2048];

bool ProcessExtensionLine(const char *line, ProjectStateContext *ctx, bool isUndo, struct project_config_extension_t *reg) // returns BOOL if line (and optionally subsequent lines) processed
{
  bool commentign=false;
  LineParser lp(commentign);
  if (lp.parse(line) || lp.getnumtokens()<1) return false;
  if (strcmp(lp.gettoken_str(0),"<NINJAMLOOP")) return false; // only look for <NINJAMLOOP lines
  
  int child_count=1;
  for (;;)
  {
    char linebuf[4096];
    if (ctx->GetLine(linebuf,sizeof(linebuf))) break;
    if (lp.parse(linebuf)||lp.getnumtokens()<=0) continue;

    if (child_count == 1)
    {
      if (lp.gettoken_str(0)[0] == '>') break;

      if (!strcmp(lp.gettoken_str(0),"LASTFN"))
      {
        lstrcpyn(g_last_insertfilename,lp.gettoken_str(1),sizeof(g_last_insertfilename)); // load config
      }
      else if (lp.gettoken_str(0)[0] == '<') child_count++; // ignore block
      else  // ignore unknown line
      {
      }
    }
    else if (lp.gettoken_str(0)[0] == '<') child_count++; // track subblocks
    else if (lp.gettoken_str(0)[0] == '>') child_count--;

  }
  return true;
}

void SaveExtensionConfig(ProjectStateContext *ctx, bool isUndo, struct project_config_extension_t *reg)
{
  ctx->AddLine("<NINJAMLOOP");
  ctx->AddLine("LASTFN \"%s\"",g_last_insertfilename);
  ctx->AddLine(">");
}

void BeginLoadProjectState(bool isUndo, struct project_config_extension_t *reg)
{
  // set defaults here (since we know that ProcessExtensionLine could come up)
}

project_config_extension_t pcreg={
  ProcessExtensionLine,
  SaveExtensionConfig,
  BeginLoadProjectState, 
  NULL,
};

// call this on startup

    rec->Register("projectconfig",&pcreg); // project configuratoin saving test
