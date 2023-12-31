// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_

#include <iterator>
#include <list>
#include <map>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"

class CXFA_LayoutItem;
class CXFA_LayoutProcessor;
class CXFA_Node;

class CXFA_ViewLayoutProcessor
    : public cppgc::GarbageCollected<CXFA_ViewLayoutProcessor> {
  CPPGC_USING_PRE_FINALIZER(CXFA_ViewLayoutProcessor, PreFinalize);

 public:
  struct BreakData {
    CPPGC_STACK_ALLOCATED();  // Raw/Unowned pointers allowed.
   public:
    CXFA_Node* pLeader;
    CXFA_Node* pTrailer;
    bool bCreatePage;
  };

  struct OverflowData {
    CPPGC_STACK_ALLOCATED();  // Raw/Unowned pointers allowed.
   public:
    CXFA_Node* pLeader;
    CXFA_Node* pTrailer;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ViewLayoutProcessor();

  void PreFinalize();
  void Trace(cppgc::Visitor* visitor) const;
  cppgc::Heap* GetHeap() const { return m_pHeap; }

  bool InitLayoutPage(CXFA_Node* pFormNode);
  bool PrepareFirstPage(CXFA_Node* pRootSubform);
  float GetAvailHeight();
  bool GetNextAvailContentHeight(float fChildHeight);
  void SubmitContentItem(CXFA_ContentLayoutItem* pContentLayoutItem,
                         CXFA_ContentLayoutProcessor::Result eStatus);
  void FinishPaginatedPageSets();
  void SyncLayoutData();
  int32_t GetPageCount() const;
  CXFA_ViewLayoutItem* GetPage(int32_t index) const;
  int32_t GetPageIndex(const CXFA_ViewLayoutItem* pPage) const;
  CXFA_ViewLayoutItem* GetRootLayoutItem() const {
    return m_pPageSetRootLayoutItem;
  }
  absl::optional<BreakData> ProcessBreakBefore(const CXFA_Node* pBreakNode);
  absl::optional<BreakData> ProcessBreakAfter(const CXFA_Node* pBreakNode);
  absl::optional<OverflowData> ProcessOverflow(CXFA_Node* pFormNode,
                                               bool bCreatePage);
  CXFA_Node* QueryOverflow(CXFA_Node* pFormNode);
  CXFA_Node* ProcessBookendLeader(const CXFA_Node* pBookendNode);
  CXFA_Node* ProcessBookendTrailer(const CXFA_Node* pBookendNode);

 private:
  class CXFA_ViewRecord : public cppgc::GarbageCollected<CXFA_ViewRecord> {
   public:
    CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
    ~CXFA_ViewRecord();

    void Trace(cppgc::Visitor* visitor) const;

    cppgc::Member<CXFA_ViewLayoutItem> pCurPageSet;
    cppgc::Member<CXFA_ViewLayoutItem> pCurPageArea;
    cppgc::Member<CXFA_ViewLayoutItem> pCurContentArea;

   private:
    CXFA_ViewRecord();
  };

  CXFA_ViewLayoutProcessor(cppgc::Heap* pHeap,
                           CXFA_LayoutProcessor* pLayoutProcessor);

  bool AppendNewPage(bool bFirstTemPage);
  void RemoveLayoutRecord(CXFA_ViewRecord* pNewRecord,
                          CXFA_ViewRecord* pPrevRecord);
  bool HasCurrentViewRecord() const {
    return m_CurrentViewRecordIter != m_ProposedViewRecords.end();
  }
  CXFA_ViewRecord* GetCurrentViewRecord() {
    return HasCurrentViewRecord() ? m_CurrentViewRecordIter->Get() : nullptr;
  }
  const CXFA_ViewRecord* GetCurrentViewRecord() const {
    return HasCurrentViewRecord() ? m_CurrentViewRecordIter->Get() : nullptr;
  }
  void ResetToFirstViewRecord() {
    m_CurrentViewRecordIter = m_ProposedViewRecords.begin();
  }
  std::list<cppgc::Member<CXFA_ViewRecord>>::iterator GetTailPosition() {
    auto iter = m_ProposedViewRecords.end();
    return !m_ProposedViewRecords.empty() ? std::prev(iter) : iter;
  }
  void AppendNewRecord(CXFA_ViewRecord* pNewRecord);
  CXFA_ViewRecord* CreateViewRecord(CXFA_Node* pPageNode, bool bCreateNew);
  CXFA_ViewRecord* CreateViewRecordSimple();
  void AddPageAreaLayoutItem(CXFA_ViewRecord* pNewRecord,
                             CXFA_Node* pNewPageArea);
  void AddContentAreaLayoutItem(CXFA_ViewRecord* pNewRecord,
                                CXFA_Node* pContentArea);
  bool RunBreak(XFA_Element eBreakType,
                XFA_AttributeValue eTargetType,
                CXFA_Node* pTarget,
                bool bStartNew);
  bool ShouldGetNextPageArea(CXFA_Node* pTarget, bool bStartNew) const;
  bool BreakOverflow(const CXFA_Node* pOverflowNode,
                     bool bCreatePage,
                     CXFA_Node** pLeaderTemplate,
                     CXFA_Node** pTrailerTemplate);
  CXFA_Node* ProcessBookendLeaderOrTrailer(const CXFA_Node* pBookendNode,
                                           bool bLeader);
  CXFA_Node* ResolveBookendLeaderOrTrailer(const CXFA_Node* pBookendNode,
                                           bool bLeader);
  absl::optional<BreakData> ProcessBreakBeforeOrAfter(
      const CXFA_Node* pBreakNode,
      bool bBefore);
  BreakData ExecuteBreakBeforeOrAfter(const CXFA_Node* pCurNode, bool bBefore);

  int32_t CreateMinPageRecord(CXFA_Node* pPageArea,
                              bool bTargetPageArea,
                              bool bCreateLast);
  void CreateMinPageSetRecord(CXFA_Node* pPageSet, bool bCreateAll);
  void CreateNextMinRecord(CXFA_Node* pRecordNode);
  bool FindPageAreaFromPageSet(CXFA_Node* pPageSet,
                               CXFA_Node* pStartChild,
                               CXFA_Node* pTargetPageArea,
                               CXFA_Node* pTargetContentArea,
                               bool bNewPage,
                               bool bQuery);
  bool FindPageAreaFromPageSet_Ordered(CXFA_Node* pPageSet,
                                       CXFA_Node* pStartChild,
                                       CXFA_Node* pTargetPageArea,
                                       CXFA_Node* pTargetContentArea,
                                       bool bNewPage,
                                       bool bQuery);
  bool FindPageAreaFromPageSet_SimplexDuplex(
      CXFA_Node* pPageSet,
      CXFA_Node* pStartChild,
      CXFA_Node* pTargetPageArea,
      CXFA_Node* pTargetContentArea,
      bool bNewPage,
      bool bQuery,
      XFA_AttributeValue ePreferredPosition);
  bool MatchPageAreaOddOrEven(CXFA_Node* pPageArea);
  CXFA_Node* GetNextAvailPageArea(CXFA_Node* pTargetPageArea,
                                  CXFA_Node* pTargetContentArea,
                                  bool bNewPage,
                                  bool bQuery);
  bool GetNextContentArea(CXFA_Node* pTargetContentArea);
  void InitPageSetMap();
  void ProcessLastPageSet();
  bool IsPageSetRootOrderedOccurrence() const {
    return m_ePageSetMode == XFA_AttributeValue::OrderedOccurrence;
  }
  void ClearData();
  void MergePageSetContents();
  void LayoutPageSetContents();
  void PrepareLayout();
  void SaveLayoutItemChildren(CXFA_LayoutItem* pParentLayoutItem);
  void ProcessSimplexOrDuplexPageSets(CXFA_ViewLayoutItem* pPageSetLayoutItem,
                                      bool bIsSimplex);

  UnownedPtr<cppgc::Heap> m_pHeap;
  cppgc::Member<CXFA_LayoutProcessor> m_pLayoutProcessor;
  cppgc::Member<CXFA_Node> m_pPageSetNode;
  cppgc::Member<CXFA_Node> m_pCurPageArea;
  cppgc::Member<CXFA_ViewLayoutItem> m_pPageSetRootLayoutItem;
  cppgc::Member<CXFA_ViewLayoutItem> m_pPageSetCurLayoutItem;
  std::list<cppgc::Member<CXFA_ViewRecord>> m_ProposedViewRecords;
  std::list<cppgc::Member<CXFA_ViewRecord>>::iterator m_CurrentViewRecordIter;
  int32_t m_nAvailPages = 0;
  int32_t m_nCurPageCount = 0;
  XFA_AttributeValue m_ePageSetMode = XFA_AttributeValue::OrderedOccurrence;
  bool m_bCreateOverFlowPage = false;
  std::map<cppgc::Member<CXFA_Node>, int32_t> m_pPageSetMap;
  std::vector<cppgc::Member<CXFA_ViewLayoutItem>> m_PageArray;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_
